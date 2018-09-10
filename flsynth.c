#include <alloca.h>// To make 'clock_gettime' and 'usleep' work

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "flsynth.h"
#include "fluidlite.h"

#ifndef __ANDROID__
#include <SDL2/SDL.h>
#else
#include <SLES/OpenSLES.h>
#include "opensl_stream/opensl_stream.h"

#include <pthread.h>
#include <semaphore.h>

#endif


void init() __attribute__((constructor));
void fini() __attribute__((destructor));


static inline void synthesize(synth_t *synth) {
    // stereo
    if (synth->channels == 2) {
        fluid_synth_write_s16(synth->fluid_synth, FRAME_PER_CYCLE, synth->buff, 0, 2, synth->buff, 1, 2);
    }
    // mono
    else {
        fluid_synth_write_s16(synth->fluid_synth, FRAME_PER_CYCLE, synth->buff, 0, 1,
                              synth->buff + FRAME_PER_CYCLE * 2, 0, 1);
        // Average the 2 channels
        int16_t *s16buff = synth->buff;
        for (int i = 0; i < FRAME_PER_CYCLE; i++) {
            s16buff[i] = (s16buff[i] >> 1) + (s16buff[i + FRAME_PER_CYCLE] >> 1);
        }
    }
}


#ifndef __ANDROID__

static int synthesize_thread(synth_t *synth) {
    int sem_res = 0;
    SDL_sem *proc_sem = synth->proc_sem;
    while (true) {
        sem_res = SDL_SemWaitTimeout(proc_sem, 500);
        // Stop thread on timeout or if synth is stopped
        if (sem_res == SDL_MUTEX_TIMEDOUT || synth->audio_device == 0) break;
        synthesize(synth);
    }
    // We need to destroy semaphore here
    SDL_DestroySemaphore(proc_sem);
    return 0;
}

static void audio_callback(synth_t *synth, unsigned char *stream, size_t stream_length) {
//    printf("Length: %zu\n", stream_length);
    // Just copy precalculated buffer
    memcpy(stream, synth->buff, stream_length);
    // Start next calculation
    SDL_SemPost(synth->proc_sem);
}

#else // __ANDROID__

static void *synthesize_thread(synth_t *synth) {
    struct timespec to = {0, 500000000};
    sem_t *proc_sem = synth->proc_sem;
    while (true) {
        int sem_res = sem_timedwait(proc_sem, &to);
        if ((synth->audio_device == 0) || (sem_res != 0 && errno == ETIMEDOUT)) break;
        synthesize(synth);
    }
    // We need to destroy semaphore here
    sem_destroy(proc_sem);
    return NULL;
}

static void audio_callback(synth_t *synth, int __unused sample_rate, int buffer_frames,
                           int __unused input_channels, const short __unused *input_buffer,
                           int output_channels, short *output_buffer) {
    // Just copy precalculated buffer
    memcpy(output_buffer, synth->buff, (size_t) (buffer_frames * output_channels * 2));
    // Start next calculation
    sem_post(synth->proc_sem);
}

#endif


synth_t *flsynth_create(unsigned int sample_rate, unsigned char channels) {
    synth_t *synth = calloc(sizeof(synth_t), 1);

    synth->settings = new_fluid_settings();
    synth->fluid_synth = new_fluid_synth(synth->settings);
    synth->buff = malloc((size_t) (FRAME_PER_CYCLE * 8)); // 8 -> 32 bit float * 2 channel (stereo)
    fluid_synth_set_sample_rate(synth->fluid_synth, sample_rate);

    synth->sample_rate = sample_rate;
    synth->channels = channels;

#ifndef __ANDROID__
    SDL_InitSubSystem(SDL_INIT_AUDIO);
#else
    synth->proc_sem = calloc(sizeof(sem_t), 1);
#endif

    return synth;
}

void flsynth_free(synth_t *synth) {
    delete_fluid_synth(synth->fluid_synth);
    delete_fluid_settings(synth->settings);

    free(synth->buff);
    free(synth);

#ifndef __ANDROID__
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
#else
    free(synth->proc_sem);
#endif
}

int flsynth_sfload(synth_t *synth, const char *filename, bool program_reset) {
    int res = fluid_synth_sfload(synth->fluid_synth, filename, program_reset);
    if (res > 0) synth->last_sfid = (unsigned int) res;
    return synth->last_sfid;
}


bool flsynth_start(synth_t *synth) {
    // Initial buffer calculation
    synthesize(synth);

#ifndef __ANDROID__
    // Create a semaphore for thread synchronization
    synth->proc_sem = SDL_CreateSemaphore(0);

    // Create audio specification
    SDL_AudioSpec audioSpec;
    audioSpec.freq = synth->sample_rate;
    audioSpec.format = AUDIO_S16LSB;
    audioSpec.channels = synth->channels;
    audioSpec.samples = FRAME_PER_CYCLE;
    audioSpec.size = 0;
    audioSpec.callback = (SDL_AudioCallback) audio_callback;
    audioSpec.userdata = synth;

    // Open audio device
    synth->audio_device = 1; SDL_OpenAudio(&audioSpec, NULL);
//    synth->audio_device = SDL_OpenAudioDevice(NULL, 0, &audioSpec, NULL, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    // Start playing
    SDL_PauseAudioDevice(synth->audio_device, 0);

    // Start background calculation
    synth->proc_thread = SDL_CreateThread((SDL_ThreadFunction) synthesize_thread, "flsynth_processing_thread", synth);
#else // __ANDROID__
    // Create a semaphore for thread synchronization
    sem_init(synth->proc_sem, 0, 0);

    // Initialize audio device and start playback
    synth->audio_device = opensl_open(synth->sample_rate, 0, synth->channels,
                                      FRAME_PER_CYCLE * 8, (opensl_process_t) audio_callback, synth);
    opensl_start(synth->audio_device);

    // Start background calculation
    pthread_create(synth->proc_thread, NULL, (void *(*)(void *)) synthesize_thread, synth);
#endif

    return true;
}

void flsynth_stop(synth_t *synth) {
#ifndef __ANDROID__
    SDL_CloseAudioDevice(synth->audio_device);
    synth->audio_device = 0;
    SDL_SemPost(synth->proc_sem);
#else // __ANDROID__
    opensl_pause(synth->audio_device);
    opensl_close(synth->audio_device);
    synth->audio_device = NULL;
    sem_post(synth->proc_sem);
#endif
}


bool flsynth_program_select_sfid(synth_t *synth, int chan, unsigned int sfid, unsigned int bank, unsigned int preset) {
    return fluid_synth_program_select(synth->fluid_synth, chan, sfid, bank, preset) == 0;
}

bool flsynth_program_select(synth_t *synth, int chan, unsigned int bank, unsigned int preset) {
    return flsynth_program_select_sfid(synth, chan, synth->last_sfid, bank, preset);
}


bool flsynth_noteon(synth_t *synth, int chan, int key, int velocity) {
    return fluid_synth_noteon(synth->fluid_synth, chan, key, velocity) == 0;
}

bool flsynth_noteoff(synth_t *synth, int chan, int key) {
    return fluid_synth_noteoff(synth->fluid_synth, chan, key) == 0;
}


bool flsynth_system_reset(synth_t *synth) {
    return fluid_synth_system_reset(synth->fluid_synth) == 0;
}


/*
 * Setup
 */

static void _dummy_log() {}


void init() {
    // Initialize SDL
#ifndef __ANDROID__
    SDL_Init(0);
#endif
    // Remove unnecessary log messages and warnings
    fluid_set_log_function(FLUID_DBG, _dummy_log, NULL);
    fluid_set_log_function(FLUID_INFO, _dummy_log, NULL);
    fluid_set_log_function(FLUID_WARN, _dummy_log, NULL);
}

void fini() {
#ifndef __ANDROID__
    SDL_Quit();
#endif
}
