#pragma once

#include <algorithm>
#include <cstdint>
#include <span>
#include <vector>

#include "types.hpp"

namespace tmp {

  template<sample_rate RATE, template<sample_rate> typename INSTRUMENT>
  class sequencer
  {

  public:
    constexpr sequencer(INSTRUMENT<RATE> &instrument)
      : m_instrument{ instrument }
    {}

    constexpr void queue_event(note note, std::uint32_t noteOnSample, std::uint32_t noteOffSample)
    {
      queue_emplace(note, noteOnSample, noteOffSample);
    }

    template<block_size BLOCK_SIZE>
    constexpr void render(std::span<float, BLOCK_SIZE.samples_per_block> buffer)
    {
      // find events to trigger for this block
      std::uint32_t blockEndSampleNumber = m_blockStartSampleNumber + BLOCK_SIZE.samples_per_block - 1;
      while (!queue_is_empty()) {

        auto &e = queue_top();
        // is the soonest event after this current block? if so we can stop looking
        if (e.noteOn > blockEndSampleNumber) {
          break;
        }

        // this event is in the block, send it to the instrument, this expects when to start
        // after the next render() block is called and the length to play
        m_instrument.play_note(e.playNote, e.noteOn - m_blockStartSampleNumber, e.noteOff - e.noteOn);
        queue_pop();
      }

      m_instrument.template render<BLOCK_SIZE>(buffer);

      // ready for next block
      m_blockStartSampleNumber += BLOCK_SIZE.samples_per_block;
    }

  private:
    struct event
    {
      note playNote;
      std::uint32_t noteOn;
      std::uint32_t noteOff;
    };

    struct event_min_sorter
    {
      constexpr auto operator()(event const &lhs, event const &rhs) -> bool
      {
        return lhs.noteOn > rhs.noteOn;
      }
    };

    [[nodiscard]] constexpr auto queue_is_empty() const -> bool
    {
      return m_eventQueueContainer.empty();
    }

    [[nodiscard]] constexpr auto queue_top() const -> event const&
    {
      return m_eventQueueContainer.front();
    }

    template<typename... ARGS>
    constexpr void queue_emplace(ARGS &&...args)
    {
      m_eventQueueContainer.emplace_back(std::forward<ARGS>(args)...);
      std::push_heap(m_eventQueueContainer.begin(), m_eventQueueContainer.end(), event_min_sorter{});
    }

    constexpr void queue_pop()
    {
      std::pop_heap(m_eventQueueContainer.begin(), m_eventQueueContainer.end(), event_min_sorter{});
      m_eventQueueContainer.pop_back();
    }


    INSTRUMENT<RATE> &m_instrument;
    std::uint32_t m_blockStartSampleNumber{ 0 };
    // min priority queue = ordered by first events to occur
    std::vector<event> m_eventQueueContainer{};
  };
}  // namespace tmp