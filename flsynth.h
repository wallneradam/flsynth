#ifndef FLSYNTH_LIBRARY_H
#define FLSYNTH_LIBRARY_H

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

#ifndef __ANDROID__
    unsigned int audio_device;
#else
    void *audio_device;
#endif
    void *synth_thread;

    void *synth_sem;
    void *cb_sem;

    bool playing;
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
bool flsynth_stop(synth_t *synth);

/**
 * Select program for channel
 * @param synth Synth object
 * @param chan Channel where we want to change program
 * @param bank The sounont bank where the pereset is
 * @param preset The instrument preset
 * @return
 */
bool flsynth_program_select(synth_t *synth, int chan, unsigned int bank, unsigned int preset);

/**
 * The same as flsynth_program_select, but you can specify soundfont id as well, if you loaded multiple fonts
 * @param synth Synth object
 * @param chan Channel where we want to change program
 * @param sfid The soundfont id which is returned by sfload call
 * @param bank The sounont bank where the pereset is
 * @param preset The instrument preset
 * @return
 */
bool flsynth_program_select_sfid(synth_t *synth, int chan, unsigned int sfid, unsigned int bank, unsigned int preset);

/**
 * Play a note on the specified channel
 * @param synth Synth object
 * @param chan Channel where the note should be played
 * @param key The key of the note (60 is the middle C)
 * @param velocity Velociy value of the note (0-127)
 * @return
 */
bool flsynth_noteon(synth_t *synth, int chan, int key, int velocity);

/**
 * Stop a previously playing note on a channel
 * @param synth Synth object
 * @param chan Channel where the not is playing
 * @param key Key of the note
 * @return
 */
bool flsynth_noteoff(synth_t *synth, int chan, int key);

/**
 * Big red panic button
 * @param synth Synth object
 * @return
 */
bool flsynth_system_reset(synth_t *synth);

/**
 * Send a MIDI controller event on a MIDI channel.
 * @param synth Synth object
 * @param chan Channel number
 * @param ctrl MIDI controller number
 * @param val MIDI controller value
 * @return
 */
bool flsynth_cc(synth_t *synth, int chan, int ctrl, int val);


#endif