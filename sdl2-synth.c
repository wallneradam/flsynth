#include "sdl2-synth.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "fluidlite.h"
#include <SDL2/SDL.h>


void init() __attribute__((constructor));
void fini() __attribute__((destructor));


static inline void synthesize(synth_t *synth) {
    // stereo
    if (synth->channels == 2) {
        fluid_synth_write_s16(synth->fluid_synth, FRAME_PER_CYCLE, synth->buff, 0, 2,
                              synth->buff, 1, 2);
    }
    // mono
    else {
        fluid_synth_write_s16(synth->fluid_synth, FRAME_PER_CYCLE, synth->buff, 0, 1,
                              synth->buff + FRAME_PER_CYCLE * 2, 0, 1);
        // TODO: average the 2 channels together
    }
}


static int synthesize_thread(synth_t *synth) {
    int sem_res = 0;
    while (true) {
        sem_res = SDL_SemWaitTimeout(synth->proc_sem, 500);
        // Stop thread on timeout or if synth is stopped
        if (sem_res == SDL_MUTEX_TIMEDOUT || synth->audio_device == 0) break;
        synthesize(synth);
    }
    return 0;
}


static void audio_callback(synth_t *synth, unsigned char *stream, size_t stream_length) {
    // Just copy precalculated buffer
    memcpy(stream, synth->buff, stream_length);
    // Start next calculation
    SDL_SemPost(synth->proc_sem);
}


synth_t *sdl_synth_create(unsigned int sample_rate, unsigned char channels) {
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    synth_t *synth = calloc(sizeof(synth_t), 1);

    synth->settings = new_fluid_settings();
    synth->fluid_synth = new_fluid_synth(synth->settings);
    synth->buff = malloc((size_t) (FRAME_PER_CYCLE * 4)); // 5 -> 16 bit stereo
    fluid_synth_set_sample_rate(synth->fluid_synth, sample_rate);

    synth->sample_rate = sample_rate;
    synth->channels = channels;

    return synth;
}

void sdl_synth_free(synth_t *synth) {
    delete_fluid_synth(synth->fluid_synth);
    delete_fluid_settings(synth->settings);

    free(synth->buff);
    free(synth);

    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

int sdl_synth_sfload(synth_t *synth, const char *filename, bool program_reset) {
    int res = fluid_synth_sfload(synth->fluid_synth, filename, program_reset);
    if (res > 0) synth->last_sfid = (unsigned int) res;
    return synth->last_sfid;
}


bool sdl_synth_start(synth_t *synth) {
    // Create audio specification
    SDL_AudioSpec audioSpec;
    audioSpec.freq = synth->sample_rate;
    audioSpec.format = AUDIO_S16;
    audioSpec.channels = synth->channels;
    audioSpec.samples = FRAME_PER_CYCLE;
    audioSpec.size = 0;
    audioSpec.callback = (SDL_AudioCallback) audio_callback;
    audioSpec.userdata = synth;

    // Initial buffer calculation
    synthesize(synth);

    // Create a semaphore for thread synchronization
    synth->proc_sem = SDL_CreateSemaphore(0);
    // Start background calculation
    synth->proc_thread = SDL_CreateThread((SDL_ThreadFunction) synthesize_thread, "sdl_synth_processing_thread", synth);

    // Open audio device
    synth->audio_device = SDL_OpenAudioDevice(NULL, 0, &audioSpec, &audioSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    // Start playing
    SDL_PauseAudioDevice(synth->audio_device, 0);
}

void sdl_synth_stop(synth_t *synth) {
    SDL_CloseAudioDevice(synth->audio_device);
    synth->audio_device = 0;
    SDL_SemPost(synth->proc_sem);
}


bool sdl_synth_program_select_sfid(synth_t *synth, int chan, unsigned int sfid, unsigned int bank, unsigned int preset) {
    return fluid_synth_program_select(synth->fluid_synth, chan, sfid, bank, preset) == 0;
}

bool sdl_synth_program_select(synth_t *synth, int chan, unsigned int bank, unsigned int preset) {
    return sdl_synth_program_select_sfid(synth, chan, synth->last_sfid, bank, preset);
}


bool sdl_synth_noteon(synth_t *synth, int chan, int key, int velocity) {
    return fluid_synth_noteon(synth->fluid_synth, chan, key, velocity) == 0;
}

bool sdl_synth_noteoff(synth_t *synth, int chan, int key) {
    return fluid_synth_noteoff(synth->fluid_synth, chan, key) == 0;
}


bool sdl_synth_system_reset(synth_t *synth) {
    return fluid_synth_system_reset(synth->fluid_synth) == 0;
}


static void _dummy_log() {}


void init() {
    // Initialize SDL
    SDL_Init(0);
    // Remove log messages
    fluid_set_log_function(FLUID_DBG, _dummy_log, NULL);
    fluid_set_log_function(FLUID_INFO, _dummy_log, NULL);
    fluid_set_log_function(FLUID_WARN, _dummy_log, NULL);
}

void fini() {
    SDL_Quit();
}

