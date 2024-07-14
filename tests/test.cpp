/*
 * You can copy code into this file and attempt to debug the constexpr code at
 * run time.
 */


#include <iostream>

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

void test_notes()
{
  using namespace std::literals;

  auto notes = std::array{
    "G#4"sv, "G4"sv, "F#4"sv, "F4"sv,
    "E4"sv, "D#4"sv, "D4"sv, "C#4"sv,
    "C4"sv, "B3"sv, "A#3"sv, "A3"sv, "G#3"sv
  };

  for(auto n : notes) {
      tmp::note parsed{n};
      std::cout << parsed.note_name << "\t"
                << parsed.note_number << "\t"
                << parsed.note_frequency.hertz << "\n";
  }
}


int main()
{
  using namespace tmp;
  using namespace tmp::literals;
  using namespace tmp::instruments;

  test_notes();


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

  return 0;
}
