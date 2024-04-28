#include <cstdint>
#include <array>

#include "wav_render.hpp"

[[gnu::section(".wavefile"), gnu::used]]
constinit auto const WaveData = []{
    tmp::wav_renderer_mono<tmp::sample_rate{8000}, tmp::seconds{5}> wav{};
    wav.render();
    return wav.data;
}();

