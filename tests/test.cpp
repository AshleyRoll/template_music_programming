#include <array>

#include "../include/tmp/sequencer.hpp"
#include "../include/tmp/synth.hpp"
#include "../include/tmp/types.hpp"
#include "../include/tmp/wav_render.hpp"

auto notesfull = [] -> std::string_view {
       return R"(
   | 1                 | 2                 | 3                 | 4                 | 5                 | 6                 | 7                 | 8                 | 9                 |
   |----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|
G#4|    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |####:    :    :    |    :    :    :    |    :    :    :    |
G4 |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
F#4|    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
F4 |    :    :    :    |## #:##  :    :    |    :    :    :    |    :    :    :    |    :    :    :    |## #:#   :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
E4 |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
D#4|    :    :    :    |    :  ##:##  :    |## #:##  :    :    |    :##  :    :    |##  :    :    :    |    :  ##:##  :    |    :    :    :    |    :##  :    :    |    :    :    :    |
D4 |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
C#4|    :    :    :  # |    :    :    :    |    :  ##:##  :  # |####:    :    :    |    :####:    :  # |    :    :    :    |    :  ##:### :  # |####:    :    :    |### :####:    :    |
C4 |    :    :    :    |    :    :    :  # |    :    :    :    |    :   #:    :    |    :    :    :    |    :    :    :  # |    :##  :    :    |    :    :    :    |    :    :    :    |
B3 |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
A#3|    :    :    : # #|    :    :    : # #|    :    :    : # #|    :  # :##  :    |    :    :    : # #|    :    :    : # #|    :    :    : # #|    :  ##:####:    |    :    :    :    |
A3 |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |    :    :    :    |
G#3|    :    :    :#   |    :    :    :#   |    :    :    :#   |    :    :  ##:  ##|    :    :    :#   |    :    :    :#   |    :    :    :#   |    :    :    :  ##|    :    :    :    |
   |----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|----+----+----+----|
)";
};
auto notes = [] -> std::string_view {
  return R"(
   | 1                 |
   |----+----+----+----|
G#4|    :    :    :    |
G4 |    :    :    :    |
F#4|    :    :    :    |
F4 |    :    :    :    |
E4 |    :    :    :    |
D#4|    :    :    :    |
D4 |    :    :    :    |
C#4|    :    :    :  # |
C4 |    :    :    :    |
B3 |    :    :    :    |
A#3|    :    :    : # #|
A3 |    :    :    :    |
G#3|    :    :    :#   |
   |----+----+----+----|
)";
};

int main()
{
  using namespace tmp;
  using namespace tmp::literals;
  using namespace tmp::instruments;

  static constexpr sample_rate rate{8192};

  triangle_synth<rate> synth{};
  sequencer sequencer{ synth };

  constexpr auto music_length = parse_music_length(notesfull);
  sequencer.parse_music(notesfull);

  wav_renderer_mono<rate, music_length> wav{};

  wav.render(sequencer);
  auto data = wav.data;
  return 0;
}