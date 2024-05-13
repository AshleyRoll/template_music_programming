#include <array>

#include "tmp/synth.hpp"
#include "tmp/types.hpp"
#include "tmp/wav_render.hpp"

[[gnu::section(".wavefile"), gnu::used]]
constinit auto const WaveData = [] {
  using namespace tmp;
  using namespace tmp::literals;

  wav_renderer_mono<sample_rate{ 4000 }, seconds{ 2.0F }, block_size{ 512 }> wav{};

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
