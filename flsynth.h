#ifndef FLSYNTH_LIBRARY_H
#define FLSYNTH_LIBRARY_H

#include <stdbool.h>

// Howmany frames are processed in one cycle,
//  if this is greater you will have more latency, but more stable sound
#define FRAME_PER_CYCLE 128


typedef struct {
    unsigned int sample_rate;
    unsigned char channels;

    void* settings;
    void* fluid_synth;
    unsigned int last_sfid;
    void *buff;

#ifndef __ANDROID__
    unsigned int audio_device;
#else
    void *audio_device;
#endif
    void *synth_thread;

    void *synth_sem;
    void *cb_sem;
} synth_t;


/**
 * Create synthesizer
 * @return A struct contains the state of the synth object
 */
synth_t *flsynth_create(unsigned int sample_rate, unsigned char channels);

/**
 * Free the synth object
 * @param synth A synth object to be free
 */
void flsynth_free(synth_t *synth);

/**
 * Load soundfont
 * Soundfont can be sf2 and sf3 (ogg vorbis) format
 * @param synth Synth object
 * @param filename Path of soundfont file
 * @param program_reset
 * @return The soundfont ID
 */
int flsynth_sfload(synth_t *synth, const char *filename, bool program_reset);

/**
 * Start synthesizer
 * Start the audio engine, the synthesis in the background, and send the result of the synth to the audio output.
 * @param synth Synth object
 * @return
 */
bool flsynth_start(synth_t *synth);

/**
 * Stop the synthesis thread and audio engine.
 * @param synth Synth object
 */
void flsynth_stop(synth_t *synth);


bool flsynth_program_select(synth_t *synth, int chan, unsigned int bank, unsigned int preset);
bool flsynth_program_select_sfid(synth_t *synth, int chan, unsigned int sfid, unsigned int bank, unsigned int preset);

bool flsynth_noteon(synth_t *synth, int chan, int key, int velocity);
bool flsynth_noteoff(synth_t *synth, int chan, int key);

bool flsynth_system_reset(synth_t *synth);


#endif