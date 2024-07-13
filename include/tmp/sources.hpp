#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <optional>
#include <span>

#include "types.hpp"

namespace tmp::sources {
  //
  // Simple SIN wave oscillator
  //
  template<sample_rate RATE>
  class sin_oscillator
  {
    static constexpr float Tau = 2.0F * std::numbers::pi_v<float>;

  public:
    constexpr sin_oscillator(frequency freq, volume vol)
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

  //
  // Simple triangle wave oscillator
  //
  template<sample_rate RATE>
  class triangle_oscillator
  {
  public:
    constexpr triangle_oscillator(frequency freq, volume vol)
      : m_incrementPerSample{ compute_slope(freq) }
      , m_volume{ vol }
    {}

    template<block_size BLOCK_SIZE>
    constexpr void render(std::span<float, BLOCK_SIZE.samples_per_block> buffer)
    {
      for (std::size_t i{ 0 }; i < BLOCK_SIZE.samples_per_block; ++i) {
        buffer[i] = next_sample();
      }
    }

  private:
    bool m_goingUp{ true };
    float m_lastLevel{ 0.0F };
    float m_incrementPerSample;
    volume m_volume;

    constexpr auto next_sample() -> float
    {
      if (m_goingUp) {
        m_lastLevel += m_incrementPerSample;
        if (m_lastLevel >= 1.0F) {
          m_goingUp = false;
        }
      } else {
        m_lastLevel -= m_incrementPerSample;
        if (m_lastLevel <= -1.0F) {
          m_goingUp = true;
        }
      }
      return m_lastLevel * m_volume.value;
    }

    static constexpr auto compute_slope(frequency frequency) -> float
    {
      return 1.0F / frequency.to_half_period().to_samples(RATE);
    }
  };

  //
  // Envelope generator. Configure then call note_on() to start the envelope
  // and call note_off() to enter the sustain ramp down to 0.0F.
  //
  // is_idle() will indicate when the envelope is complete.
  //
  //          /\
  //         /  \
  //        /   --- . . . ---\
  //    ___/                  \___
  //    w  a  d  s           r i
  //
  //  on --^                 ^-- off
  //
  // w = Wait for note_on
  // a = Attack note_on time
  // d = Decay
  // s = Sustain
  // r = Release (note_off time)
  // i = Idle (all values are 0.0f)
  //
  template<sample_rate RATE>
  class envelope
  {
  public:
    constexpr envelope(seconds attackTime,
      volume attackLevel,
      seconds decayTime,
      volume decayLevel,
      seconds releaseTime)
      : m_deltas({
          { 0.0F, 0.0F }, // wait / idle
          { compute_slope(volume{ 0.0F }, attackLevel, attackTime), attackLevel.value }, // Attack
          { compute_slope(attackLevel, decayLevel, decayTime), decayLevel.value }, // Decay
          { 0.0F, decayLevel.value }, // Sustain
          { compute_slope(decayLevel, volume{ 0.0F }, releaseTime), 0.0F }  // Release
    })
    {}

    [[nodiscard]] constexpr auto is_idle() const
    {
      return m_state == State::Idle;
    }

    // begin the note envelope in the given number of samples from the start of the next apply() block
    constexpr void note_on(std::uint32_t startNumberSamplesFromNextBlock, std::uint32_t stopAfterNumberSamples)
    {
      m_sampleCounter = startNumberSamplesFromNextBlock;
      m_offSampleCounter = stopAfterNumberSamples;
      m_state = State::Wait;
    }

    template<block_size BLOCK_SIZE>
    constexpr void apply(std::span<float, BLOCK_SIZE.samples_per_block> buffer)
    {
      for (std::size_t i{ 0 }; i < BLOCK_SIZE.samples_per_block; ++i) {
        // Wait -> Attack [note on]
        // Attack -> Release [note off]
        // Decay -> Release [note off]
        // Sustain -> Release [note off]
        // Release -> Idle

        // otherwise apply the current delta
        // check for state increment if current matches delta target
        switch (m_state) {
        case State::Wait:
        case State::Idle:
          buffer[i] = 0.0F;  // silence
          if (handle_note_command(State::Attack)) {  // note on?
            // we know the note length so configure the counter
            // to trip to release after that time.
            m_sampleCounter = m_offSampleCounter;
          }
          break;

        case State::Attack:
          buffer[i] *= handle_up_ramp(State::Decay);
          handle_note_command(State::Release);  // note off?
          break;

        case State::Decay:
          buffer[i] *= handle_down_ramp(State::Sustain);
          handle_note_command(State::Release);  // note off?
          break;

        case State::Sustain:
          buffer[i] *= handle_sustain();
          handle_note_command(State::Release);  // note off?
          break;

        case State::Release:
          buffer[i] *= handle_down_ramp(State::Idle);
          break;
        }
      }
    }

  private:
    enum class State : std::uint8_t { Wait = 0, Attack = 1, Decay = 2, Sustain = 3, Release = 4, Idle = 5 };

    struct delta
    {
      float increment_per_sample;
      float target_value;
    };

    State m_state{ State::Wait };
    float m_lastLevel{ 0.0F };
    std::optional<std::uint32_t> m_sampleCounter{};
    std::uint32_t m_offSampleCounter{};
    std::array<delta, 5> m_deltas;

    constexpr auto handle_note_command(State nextState) -> bool
    {
      if (m_sampleCounter.has_value()) {
        if (m_sampleCounter.value() == 0) {
          // yes, time to change state
          m_state = nextState;
          m_sampleCounter.reset();
          return true;
        } else {
          // keep waiting for state change
          --m_sampleCounter.value();
        }
      }
      return false;
    }

    constexpr auto handle_up_ramp(State nextState) -> float
    {
      auto delta = get_delta();
      m_lastLevel += delta.increment_per_sample;

      if (m_lastLevel >= delta.target_value) {
        m_state = nextState;
      }

      return m_lastLevel;
    }

    constexpr auto handle_down_ramp(State nextState) -> float
    {
      auto delta = get_delta();
      m_lastLevel += delta.increment_per_sample;

      if (m_lastLevel <= delta.target_value) {
        m_state = nextState;
      }

      return m_lastLevel;
    }

    constexpr auto handle_sustain() -> float
    {
      auto delta = get_delta();
      return delta.target_value;
    }

    constexpr auto get_delta() -> delta const &
    {
      int index = static_cast<int>(m_state);
      if (m_state == State::Idle) {
        index = 0;
      }  // reuse wait
      return m_deltas[index];
    }

    constexpr auto compute_slope(volume startLevel, volume targetLevel, seconds time) -> float
    {
      auto delta = targetLevel.value - startLevel.value;
      return delta / time.to_samples(RATE);
    }
  };

  template<sample_rate RATE, template<sample_rate> typename SOURCE>
  class note_base
  {
  public:
    template<typename... ARGS>
    constexpr explicit note_base(envelope<RATE> envelope,
      std::uint32_t startNumberSamplesFromNextBlock,
      std::uint32_t stopAfterNumberSamples,
      ARGS &&...args)
      : m_envelope{ envelope }
      , m_source{ std::forward<ARGS>(args)... }
    {
      m_envelope.note_on(startNumberSamplesFromNextBlock, stopAfterNumberSamples);
    }

    template<block_size BLOCK_SIZE>
    constexpr void render(std::span<float, BLOCK_SIZE.samples_per_block> buffer)
    {
      m_source.template render<BLOCK_SIZE>(buffer);
      m_envelope.template apply<BLOCK_SIZE>(buffer);
    }


    [[nodiscard]] constexpr auto is_idle() const
    {
      return m_envelope.is_idle();
    }

    // begin the note envelope in the given number of samples from the start of the next apply() block
    constexpr void note_on(std::uint32_t inNumberSamples, std::uint32_t offAfterNumberSamples)
    {
      m_envelope.note_on(inNumberSamples, offAfterNumberSamples);
    }

  private:
    envelope<RATE> m_envelope;
    SOURCE<RATE> m_source;
  };

}  // namespace tmp::sources
