#include <stdio.h>
#include <unistd.h>
#include "../sdl2-synth.h"

#define echo(...) printf(__VA_ARGS__); fflush(stdout)

int main() {
    synth_t *synth = sdl_synth_create(44100, 2);

    echo("Loading soundfont... ");
    sdl_synth_sfload(synth, "2gmgsmt.sf2", false);
    echo("OK.\n");
    sdl_synth_program_select(synth, 0, 0, 19); // Church organ

    echo("Playing... ");

    sdl_synth_noteon(synth, 0, 60, 127);
    sdl_synth_noteon(synth, 0, 64, 127);
    sdl_synth_noteon(synth, 0, 67, 127);

    sdl_synth_program_select(synth, 1, 0, 0); // Piano

    sdl_synth_noteon(synth, 1, 64, 127);
    sdl_synth_noteon(synth, 1, 67, 127);
    sdl_synth_noteon(synth, 1, 72, 127);

    sdl_synth_start(synth);
    sleep(5);
    sdl_synth_stop(synth);

    echo("end.\n");

    sdl_synth_free(synth);
    return 0;
}