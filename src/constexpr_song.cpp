#include <array>
#include <cstdint>

#include "tmp/wav_render.hpp"

[[gnu::section(".wavefile"), gnu::used]]
constinit auto const WaveData = [] {
  using namespace tmp;

  wav_renderer_mono<sample_rate{ 4000 }, seconds{ 2.0F }, block_size{ 512 }> wav{};

  instruments::synth<wav.Rate> synth{};
  synth.play_note(note{"C4"}, seconds{0.1F});
  synth.play_note(note{"D#4"}, seconds{0.5F});
  synth.play_note(note{"G4"}, seconds{1.0F});

  //auto source = mixer<wav.Rate, instruments::synth>{
  //  synth
  //};

  wav.render(synth);
  return wav.data;
}();
