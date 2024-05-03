#pragma once

#include <algorithm>
#include <cstdint>
#include <numbers>
#include <span>
#include <tuple>


#include "types.hpp"

namespace tmp {
  namespace sources {
    template<sample_rate RATE>
    class oscillator
    {
      static constexpr float Tau = 2.0F * std::numbers::pi_v<float>;

    public:
      constexpr oscillator(frequency freq, volume vol)
        : m_deltaTheta{ (2.0F * std::numbers::pi_v<float> * freq.hertz) / RATE.samples_per_second }
        , m_volume{ vol }
      {}

      template<block_size BLOCK_SIZE>
      constexpr void render(std::span<float, BLOCK_SIZE.samples_per_block> buffer)
      {
        for (auto &sample : buffer) {
          sample = m_volume.value * std::sin(m_theta);
          m_theta += m_deltaTheta;
        }

        // keep theta between 0..Tau (2 pi)
        while (m_theta > Tau) {
          m_theta -= Tau;
        }
      }

    private:
      float m_deltaTheta;
      volume m_volume;
      float m_theta{ 0.0F };
    };
  }  // namespace sources

  template<sample_rate RATE, template<sample_rate> typename... SOURCES>
  class mixer
  {
  public:
    using SourceTuple = std::tuple<SOURCES<RATE>...>;

    constexpr explicit mixer(SOURCES<RATE>... srcs)
      : m_sources{ srcs... }
    {}

    template<block_size BLOCK_SIZE>
    constexpr void render(std::span<float, BLOCK_SIZE.samples_per_block> buffer)
    {
      std::ranges::fill(buffer, 0.0F);  // zero the output buffer before rendering

      std::apply(
        [&buffer](auto &...sources) {
          auto process =
            [&buffer](auto &src) {
              std::array<float, BLOCK_SIZE.samples_per_block> sampleBuffer;
              src.template render<BLOCK_SIZE>(sampleBuffer);
              for (std::size_t i{ 0 }; i < BLOCK_SIZE.samples_per_block; ++i) {
                buffer[i] = std::clamp(buffer[i] + sampleBuffer[i], -1.0F, 1.0F);
              }
            };

          // fold over all sources
          (process(sources), ...);
        },
        m_sources);
    }


  private:
    SourceTuple m_sources;
  };

}  // namespace tmp