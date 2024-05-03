#pragma once

#include <cstdint>
#include <span>

namespace tmp {

  struct sample_rate
  {
    std::uint32_t samples_per_second;
  };

  struct seconds
  {
    std::uint32_t seconds;
  };

  struct frequency
  {
    float hertz;
  };

  struct volume
  {
    constexpr explicit volume(float value)
      : value{ std::clamp(value, 0.0f, 1.0f) }
    {}

    float value;
  };

  struct block_size
  {
    std::size_t samples_per_block;
  };
}  // namespace tmp