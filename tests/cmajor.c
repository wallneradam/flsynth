#include <stdio.h>
#include <unistd.h>
#include "../flsynth.h"

#define echo(...) printf(__VA_ARGS__); fflush(stdout)

int main() {
    synth_t *synth = flsynth_create(44100, 2);

    echo("Loading soundfont... ");
    flsynth_sfload(synth, "2gmgsmt.sf2", false);
    echo("OK.\n");
    flsynth_program_select(synth, 0, 0, 19); // Church organ

    echo("Playing... ");

    flsynth_noteon(synth, 0, 60, 127);
    flsynth_noteon(synth, 0, 64, 127);
    flsynth_noteon(synth, 0, 67, 127);

    flsynth_program_select(synth, 1, 0, 0); // Piano

    flsynth_noteon(synth, 1, 64, 127);
    flsynth_noteon(synth, 1, 67, 127);
    flsynth_noteon(synth, 1, 72, 127);

    flsynth_start(synth);
    sleep(5);
    flsynth_stop(synth);

    echo("end.\n");

    flsynth_free(synth);
    return 0;
}