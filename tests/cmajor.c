#include <stdio.h>
#include <unistd.h>
#include "../flsynth.h"

#define echo(...) printf(__VA_ARGS__); fflush(stdout)

int main() {
    synth_t *synth = flsynth_create(44100, 2);

    echo("Loading soundfont... ");
    flsynth_sfload(synth, "../tests/2gmgsmt.sf2", false);
    echo("OK.\n");

    echo("Playing... ");
    flsynth_start(synth);

    flsynth_program_select(synth, 0, 0, 19); // Church organ

    flsynth_noteon(synth, 0, 60, 127);
    flsynth_noteon(synth, 0, 64, 127);
    flsynth_noteon(synth, 0, 67, 127);

    flsynth_program_select(synth, 1, 0, 0); // Piano

    usleep(30000);
    flsynth_noteon(synth, 1, 64, 127);
    usleep(30000);
    flsynth_noteon(synth, 1, 67, 127);
    usleep(30000);
    flsynth_noteon(synth, 1, 72, 127);

    sleep(4);

    flsynth_noteoff(synth, 0, 60);
    flsynth_noteoff(synth, 0, 64);
    flsynth_noteoff(synth, 0, 67);

    flsynth_noteoff(synth, 1, 64);
    flsynth_noteoff(synth, 1, 67);
    flsynth_noteoff(synth, 1, 72);

    usleep(500000);

    flsynth_stop(synth);

    echo("end.\n");

    flsynth_free(synth);
    return 0;
}