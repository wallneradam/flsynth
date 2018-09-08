#ifndef SDL2_SYNTH_LIBRARY_H
#define SDL2_SYNTH_LIBRARY_H

#include <stdbool.h>

// Howmany frames are processed in one cycle,
//  if this is greater you will have more latency, but more stable sound
#define FRAME_PER_CYCLE 256

typedef struct {
    unsigned int sample_rate;
    unsigned char channels;

    void* settings;
    void* fluid_synth;
    unsigned int last_sfid;
    void *buff;

    unsigned int audio_device;

    void *proc_thread;
    void *proc_sem;
} synth_t;


/**
 * Create synthesizer
 * @return A struct contains the state of the synth object
 */
synth_t *sdl_synth_create(unsigned int sample_rate, unsigned char channels);

/**
 * Free the synth object
 * @param synth A synth object to be free
 */
void sdl_synth_free(synth_t *synth);

/**
 * Load soundfont
 * Soundfont can be sf2 and sf3 (ogg vorbis) format
 * @param synth Synth object
 * @param filename Path of soundfont file
 * @param program_reset
 * @return
 */
int sdl_synth_sfload(synth_t *synth, const char *filename, bool program_reset);

bool sdl_synth_program_select(synth_t *synth, int chan, unsigned int bank, unsigned int preset);
bool sdl_synth_program_select_sfid(synth_t *synth, int chan, unsigned int sfid, unsigned int bank, unsigned int preset);

bool sdl_synth_noteon(synth_t *synth, int chan, int key, int velocity);
bool sdl_synth_noteoff(synth_t *synth, int chan, int key);

bool sdl_synth_system_reset(synth_t *synth);

bool sdl_synth_start(synth_t *synth);
void sdl_synth_stop(synth_t *synth);

#endif