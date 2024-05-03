#include <array>
#include <cstdint>

#include "tmp/wav_render.hpp"

[[gnu::section(".wavefile"), gnu::used]]
constinit auto const WaveData = [] {
  using namespace tmp;

  wav_renderer_mono<sample_rate{ 4000 }, seconds{ 2 }, block_size{ 1024 }> wav{};

  auto source = mixer<wav.Rate, sources::oscillator, sources::oscillator, sources::oscillator>{
    { tmp::frequency{ 440.0F }, tmp::volume{ 0.3f } },
    { tmp::frequency{ 523.3F }, tmp::volume{ 0.3f } },
    { tmp::frequency{ 659.3F }, tmp::volume{ 0.3f } }
  };

  wav.render(source);
  return wav.data;
}();
