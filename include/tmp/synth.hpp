#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <vector>

#include "sources.hpp"
#include "types.hpp"

namespace tmp {
  namespace instruments {
    //
    // A simple synthesiser using sine wave oscillators
    //
    template<sample_rate RATE, template<sample_rate> typename OSCILLATOR>
    class synth_base
    {
    public:
      template<block_size BLOCK_SIZE>
      constexpr void render(std::span<float, BLOCK_SIZE.samples_per_block> buffer)
      {
        std::ranges::fill(buffer, 0.0F);  // zero the output buffer before rendering

        for (auto &note : m_playingNotes) {
          std::array<float, BLOCK_SIZE.samples_per_block> sampleBuffer;
          note.template render<BLOCK_SIZE>(sampleBuffer);
          for (std::size_t i{ 0 }; i < BLOCK_SIZE.samples_per_block; ++i) {
            buffer[i] = buffer[i] + sampleBuffer[i];
          }
        }

        // remove idle notes
        std::erase_if(m_playingNotes, [](auto &note) { return note.is_idle(); });
      }

      constexpr void
        play_note(note note, std::uint32_t startNumberSamplesFromNextBlock, std::uint32_t stopAfterNumberSamples)
      {
        using namespace literals;

        m_playingNotes.emplace_back(sources::envelope<RATE>{ 0.005_sec, 0.0_dBfs, 0.01_sec, -3.0_dBfs, 0.005_sec },
          startNumberSamplesFromNextBlock,
          stopAfterNumberSamples,
          note.note_frequency,
          -3.0_dBfs);
      }

    private:
      using Note = sources::note_base<RATE, OSCILLATOR>;

      std::vector<Note> m_playingNotes{};
    };

    template<sample_rate RATE>
    class sin_synth : public synth_base<RATE, sources::sin_oscillator>
    {
    };

    template<sample_rate RATE>
    class triangle_synth : public synth_base<RATE, sources::triangle_oscillator>
    {
    };


  }  // namespace instruments

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
          auto process = [&buffer](auto &src) {
            std::array<float, BLOCK_SIZE.samples_per_block> sampleBuffer;
            src.template render<BLOCK_SIZE>(sampleBuffer);
            for (std::size_t i{ 0 }; i < BLOCK_SIZE.samples_per_block; ++i) {
              buffer[i] = buffer[i] + sampleBuffer[i];
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