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
      constexpr explicit synth_base(envelope env, volume vol)
        : m_envelope{ env }
        , m_volume{ vol }
      {}

      template<block_size BLOCK_SIZE>
      constexpr void render(std::span<float, BLOCK_SIZE.samplesPerBlock> buffer)
      {
        std::ranges::fill(buffer, 0.0F);  // zero the output buffer before rendering

        for (auto &note : m_playingNotes) {
          std::array<float, BLOCK_SIZE.samplesPerBlock> sampleBuffer;
          note.template render<BLOCK_SIZE>(sampleBuffer);
          for (std::size_t i{ 0 }; i < BLOCK_SIZE.samplesPerBlock; ++i) {
            buffer[i] = buffer[i] + sampleBuffer[i];
          }
        }

        // remove idle music
        std::erase_if(m_playingNotes, [](auto &note) { return note.is_idle(); });
      }

      constexpr void play_note(note note, std::uint32_t startSamplesFromNextBlock, std::uint32_t stopAfterSamples)
      {
        using namespace literals;

        m_playingNotes.emplace_back(
          m_envelope, startSamplesFromNextBlock, stopAfterSamples, note.note_frequency, m_volume);
      }

    private:
      using Note = sources::note_base<RATE, OSCILLATOR>;
      envelope m_envelope;
      volume m_volume;
      std::vector<Note> m_playingNotes{};
    };

    template<sample_rate RATE>
    class sin_synth : public synth_base<RATE, sources::sin_oscillator>
    {
    public:
      constexpr sin_synth(envelope env, volume vol)
        : synth_base<RATE, tmp::sources::sin_oscillator>(env, vol)
      {}
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
    constexpr void render(std::span<float, BLOCK_SIZE.samplesPerBlock> buffer)
    {
      std::ranges::fill(buffer, 0.0F);  // zero the output buffer before rendering

      std::apply(
        [&buffer](auto &...sources) {
          auto process = [&buffer](auto &src) {
            std::array<float, BLOCK_SIZE.samplesPerBlock> sampleBuffer;
            src.template render<BLOCK_SIZE>(sampleBuffer);
            for (std::size_t i{ 0 }; i < BLOCK_SIZE.samplesPerBlock; ++i) {
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