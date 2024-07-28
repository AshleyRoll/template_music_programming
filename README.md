# TMP - Template ~~Meta~~ Music Programming

or **Constexpr Composition**

Generating music with C++ templates and compile time programming.

## Build

CMake support thanks to @seppeon

To build:

```sh
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build . --target song
$ play song.wav
```

## Songs

Add source files in the `src` folder and edit the _CMakeLists.txt_
adding a new entry with the `add_wav(name src/name.cpp)` macro.

## Writing Music

The parser for the music is not that robust and will check a few things.

Copy the format from `src/song.cpp`.

* Current _4/4_ time is assumed, and divided into 16th notes
* Empty lines are discarded
* Lines that begin with spaces are ignored
* Attempts to deals with unix vs dos end of lines
* `|` and `:` are used for visually dividing, they are ignored when generating
  events
* Lines must start with 2 or 3 character note name `[A-G]{optional #}[octave 0-8]`
* Lines must follow note name with a `|` in exactly the 4th column
* Lines must end with a `|`
* All Lines must be the same length
* A space (` `) represents no note played in that 16th
* A `#` represents a note played or continued in that 16th

## Instruments

Currently, there is only a sine wave oscillator and this is wrapped
into `tmp::instruments::sin_synth`.

It is possible to combine oscillators and synths with a `tmp::mixer`
and in theory each synth could be driven by a different `tmp::sequencer`.

Be careful about volume levels, especially when mixing (additive) so
the signal doesn't exceed `+/-1.0F`. The output will be clamped
and cause audio artifacts.

## Test

There is a `tests/test.cpp` file that can be used to build a run-time
debuggable version of code if you want to step through or check something.

The CMake target for this is `runtime-test`. Note that some of the code is
still evaluated at compile time.
