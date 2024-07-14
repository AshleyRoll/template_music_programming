#include <array>

#include "tmp/sequencer.hpp"
#include "tmp/synth.hpp"
#include "tmp/types.hpp"
#include "tmp/wav_render.hpp"

auto musicSource = [] -> tmp::music {
  return tmp::music{ tmp::beats_per_minute{ 120 },
    R"(
   | 1                 | 2                 | 3                 | 4                 | 5                 | 6                 | 7                 | 8                 |
   |----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|
G#4|    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :####:    :    |    :    :    :    |    :    :    :    |
G4 |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
F#4|    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
F4 |    :## #:##  :    |    :    :    :    |    :    :    :    |    :    :    :    |    :## #:#   :    |    :    :    :    |    :    :    :    |    :    :    :    |
E4 |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
D#4|    :    :  ##:##  |    :## #:##  :    |    :    :##  :    |    :##  :    :    |    :    :  ##:##  |    :    :    :    |    :    :##  :    |    :    :    :    |
D4 |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
C#4|  # :    :    :    |    :    :  ##:##  |  # :####:    :    |    :    :####:    |  # :    :    :    |    :    :  ##:### |  # :####:    :    |    :### :####:    |
C4 |    :    :    :    |  # :    :    :    |    :    :   #:    |    :    :    :    |    :    :    :    |  # :    :##  :    |    :    :    :    |    :    :    :    |
B3 |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
A#3| # #:    :    :    | # #:    :    :    | # #:    :  # :##  |    :    :    :    | # #:    :    :    | # #:    :    :    | # #:    :  ##:####|    :    :    :    |
A3 |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
G#3|#   :    :    :    |#   :    :    :    |#   :    :    :  ##|  ##:    :    :    |#   :    :    :    |#   :    :    :    |#   :    :    :    |  ##:    :    :    |
   |----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|
)" };
};


[[gnu::section(".wavefile"), gnu::used]]
constinit auto const WaveData = [] {
  using namespace tmp;
  using namespace tmp::literals;
  using namespace tmp::instruments;

  //static constexpr sample_rate Rate{ 8'192 };
  static constexpr sample_rate Rate{ 8'000 };

  sin_synth<Rate> synth{
    envelope{ 0.005_sec, 0.0_dBfs, 0.02_sec, -3.0_dBfs, 0.005_sec },
    -1.0_dBfs
  };
  sequencer sequencer{ synth };

  static constexpr auto music_length = parse_music_length(musicSource);
  sequencer.parse_music(musicSource);

  wav_renderer_mono<Rate, music_length> wav{};

  wav.render(sequencer);
  return wav.data;
}();
