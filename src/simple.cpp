#include <array>

#include "tmp/sequencer.hpp"
#include "tmp/synth.hpp"
#include "tmp/types.hpp"
#include "tmp/wav_render.hpp"

[[gnu::section(".wavefile"), gnu::used]]
constinit auto const WaveData = [] {
  using namespace tmp;
  using namespace tmp::literals;
  using namespace tmp::instruments;


  static constexpr sample_rate Rate{ 8'192 };

  sin_synth<Rate> synth{
    envelope{ 0.005_sec, 0.0_dBfs, 0.01_sec, -3.0_dBfs, 0.005_sec },
    -1.0_dBfs
  };

  sequencer sequencer{ synth };

  wav_renderer_mono<Rate, seconds{ 1.6F }> wav{};
  synth.play_note("A4"_note, (0.1_sec).to_samples(wav.Rate), (1.5_sec).to_samples(wav.Rate));

  wav.render(sequencer);
  return wav.data;
}();
