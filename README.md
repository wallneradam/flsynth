# SDL2 Synthesizer

It is a software MIDI synthesizer library using FluidLite as synth engine
and SDL2 as audio engine. It can load soundfont files (sf2 and sf3) 
and use their instruments as sound source.

FluidLite is a very good alternative for FluidSynth, but it has no
audio capabilities. SDL2 is a cross platform library with good audio.
So I just combined the two of them.

FluidSynth is GPL licensed, so it is hard to use in commercial
applications especially on mobile phones. FluidLite is MIT licensed 
as this library, so you are free to integrate it in any project.

SDL2-Synth contains FluidLite, so you don't need to install it 
externally. But SDL2 is needed separately as shared library 
(most Linux distributions have them installed by default).
