#include <array>

#include "tmp/synth.hpp"
#include "tmp/types.hpp"
#include "tmp/wav_render.hpp"

// Start #56 G#3, end #68 G#4
auto notes = [] -> std::string_view {
  return R"(
   | 1                 | 2                 | 3                 | 4                 | 5                 | 6                 | 7                 | 8                 | 9                 |
   |----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|
G#4:    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |####:    :    :    |    :    :    :    |    :    :    :    |
G4 :    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
F#4:    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
F4 :    :    :    :    |## #:##  :    :    |    :    :    :    |    :    :    :    |    :    :    :    |## #:#   :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
E4 :    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
D#4:    :    :    :    |    :  ##:##  :    |## #:##  :    :    |    :##  :    :    |##  :    :    :    |    :  ##:##  :    |    :    :    :    |    :##  :    :    |    :    :    :    |
D4 :    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
C#4:    :    :    :  # |    :    :    :    |    :  ##:##  :  # |####:    :    :    |    :####:    :  # |    :    :    :    |    :  ##:### :  # |####:    :    :    |### :####:    :    |
C4 :    :    :    :    |    :    :    :  # |    :    :    :    |    :   #:    :    |    :    :    :    |    :    :    :  # |    :##  :    :    |    :    :    :    |    :    :    :    |
B3 :    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
A#3:    :    :    : # #|    :    :    : # #|    :    :    : # #|    :  # :##  :    |    :    :    : # #|    :    :    : # #|    :    :    : # #|    :  ##:####:    |    :    :    :    |
A3 :    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
G#3:    :    :    :#   |    :    :    :#   |    :    :    :#   |    :    :  ##:  ##|    :    :    :#   |    :    :    :#   |    :    :    :#   |    :    :    :  ##|    :    :    :    |
   |----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|
  )";
};


[[gnu::section(".wavefile"), gnu::used]]
constinit auto const WaveData = [] {
  using namespace tmp;
  using namespace tmp::literals;

  wav_renderer_mono<sample_rate{ 4000 }, seconds{ 1.0f * 2.0F }> wav{};

  instruments::synth<wav.Rate> synth{};
  synth.play_note("C4"_note, 0.1_sec);
  synth.play_note("E4"_note, 0.5_sec);
  synth.play_note("G4"_note, 1.0_sec);

  // auto source = mixer<wav.Rate, instruments::synth>{
  //   synth
  // };

  wav.render(synth);
  return wav.data;
}();


// TODO:
// - Convert to MIDI note number. 69 == A4 == 440. Piano note number 49
// - Create vertical piano roll specifying first note number (and compute last)
// - Specify BPM and smallest note quantisation (16th)
// - convert song MIDI to piano roll
// - Read piano roll for note start and stop times and add them to instrument
//
// 120 BPM - 1 bar = 60/BPM = 60/120 = 0.5s 1 beat
// 4/4 - 4 beats to bar = 2s per bar = 2000ms per bar
// 1/16 note = bar len/16 = 2000ms / 16 = 125ms
//