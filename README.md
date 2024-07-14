# TMP - Template Music Programming

or **Constexpr Composition**

Generating music with C++ templates and compile time programming.

## Build

To build:

```sh
$ make
```

To clean:

```sh
$ make clean
```

To play (invokes `play` command):

```sh
$ make play-song
```

Where `song` is the name (without `.cpp` extension) of the source file
that generates the song.

## Songs

Add source files in the `src` folder and they will automatically be
added to the build.

This will also create a build target `play-<filename>` that will build
and play the generated audio.

* `src/song.cpp` -> `bin/song.wav` (`make play-song`)
* `src/simple.cpp` -> `bin/simple.wav` (`make play-simple`)

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

## Test

There is a `tests/test.cpp` file that can be used to build a run-time
debuggable version of code if you want to step through or check something.

This can sometimes also render parts at compile time..

```sh
$ make test
```

Will build that file into an executable `bin/test`.
