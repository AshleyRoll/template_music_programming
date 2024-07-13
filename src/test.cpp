#include <array>

#include "tmp/synth.hpp"
#include "tmp/types.hpp"
#include "tmp/wav_render.hpp"

[[gnu::section(".wavefile"), gnu::used]]
constinit auto const WaveData = [] {
  using namespace tmp;
  using namespace tmp::literals;

  wav_renderer_mono<sample_rate{ 4000 }, seconds{ 1.0f }> wav{};

  instruments::synth<wav.Rate> synth{};
  synth.play_note("C4"_note, 0.1_sec);

  wav.render(synth);
  return wav.data;
}();

