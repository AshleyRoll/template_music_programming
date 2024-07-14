#include <array>

#include "tmp/sequencer.hpp"
#include "tmp/synth.hpp"
#include "tmp/types.hpp"
#include "tmp/wav_render.hpp"

auto notes = [] -> std::string_view {
  return R"(
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
)";
};


[[gnu::section(".wavefile"), gnu::used]]
constinit auto const WaveData = [] {
  using namespace tmp;
  using namespace tmp::literals;
  using namespace tmp::instruments;

  //static constexpr sample_rate rate{8'192};
  //static constexpr sample_rate rate{16'384};
  static constexpr sample_rate rate{22'050};

  sin_synth<rate> synth{};
  sequencer sequencer{ synth };

  static constexpr auto music_length = parse_music_length(notes);
  sequencer.parse_music(notes);

  wav_renderer_mono<rate, music_length> wav{};

  wav.render(sequencer);
  return wav.data;
}();
