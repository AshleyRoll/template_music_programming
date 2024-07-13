#include <array>

#include "tmp/sequencer.hpp"
#include "tmp/synth.hpp"
#include "tmp/types.hpp"
#include "tmp/wav_render.hpp"

[[gnu::section(".wavefile"), gnu::used]]
constinit auto const WaveData = [] {
  using namespace tmp;
  using namespace tmp::literals;

  wav_renderer_mono<sample_rate{ 4000 }, seconds{ 1.0f }> wav{};

  instruments::sin_synth<wav.Rate> synth{};

  sequencer sequencer{ synth };

  synth.play_note("C4"_note, (0.1_sec).to_samples(wav.Rate), (1.0_sec).to_samples(wav.Rate));

  wav.render(sequencer);
  return wav.data;
}();
