# FLSynth

It is a multiplatform software MIDI synthesizer library using **FluidLite** as synth engine.
It uses **OpenSL ES** on Android and **SDL2** library on other platforms as audio 
engine. It can load soundfont files (sf2 and sf3) and use their instrument
prefixes as sound source.

FluidLite is a very good alternative for *FluidSynth*, but it has no
audio capabilities built in. SDL2 is a cross platform library with good audio engine.
Unfortunately SDL2 sound has too much latency on Android, so we use OpenSL ES there.

FluidSynth was GPL licensed, so it was hard to use in commercial
applications especially on mobile phones. FluidLite is MIT licensed 
as this library, so you are free to integrate it in any project.

FLSynth contains FluidLite, so you don't need to install it 
externally. But SDL2 is needed separately as shared library 
(most Linux distributions have it installed by default).

## Usage

```C
// Create synthesizer
synth_t *synth = flsynth_create(44100, 2);
// Load soundfont
flsynth_sfload(synth, "2gmgsmt.sf2", false);
flsynth_program_select(synth, 1, 0, 19); // Church organ
// C Major
flsynth_noteon(synth, 0, 60, 127);
flsynth_noteon(synth, 0, 64, 127);
flsynth_noteon(synth, 0, 67, 127);
// Start synthesis
flsynth_start(synth);
// Wait a bit
sleep(2);
flsynth_noteoff(synth, 0, 60);
flsynth_noteoff(synth, 0, 64);
flsynth_noteoff(synth, 0, 67);
// Wait for release
sleep(1);
// Stop synthethiser
flsynth_stop(synth);
// Free object
flsynth_free(synth);
```

For more information see *flsynth.h* and tests.
