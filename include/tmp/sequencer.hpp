#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "types.hpp"

namespace tmp {

  namespace detail {
    class parser
    {
    public:
      constexpr parser(beats_per_minute bpm, std::string_view music)
        : m_bpm{ bpm }
        , m_beat{ 60.0F / static_cast<float>(m_bpm.rate) }
        , m_bar{ m_beat.period * 4.0F }
        , m_16th{ m_bar.period / 16.0F }
        , m_noteLines{ extract_note_lines(music) }
      {}

      constexpr auto length() -> seconds
      {
        // cut the "C4 |"
        auto notes = m_noteLines[0].substr(4);
        auto sixteenths = std::count_if(notes.begin(), notes.end(), [](auto c) { return c == ' ' or c == '#'; });
        return seconds{ m_16th.period * static_cast<float>(sixteenths) };
      }

      constexpr void parse_events(auto insert)
      {
        using namespace tmp::literals;

        for (auto l : m_noteLines) {
          auto noteName = l[2] == ' ' ? l.substr(0, 2) : l.substr(0, 3);
          auto notes = l.substr(4);
          parse_note_events(noteName, notes, insert);
        }
      }

    private:
      beats_per_minute m_bpm;
      seconds m_beat;
      seconds m_bar;
      seconds m_16th;
      std::vector<std::string_view> m_noteLines;

      constexpr void parse_note_events(std::string_view noteName, std::string_view line, auto insert)
      {
        bool foundNoteStart{ false };
        std::size_t start16th{ 0 };
        std::size_t current16th{ 0 };
        for (auto c : line) {
          if (c == '|' or c == ':') continue;

          if (foundNoteStart) {
            if (c != '#') {
              // note ended
              insert(note{ noteName },
                seconds{ static_cast<float>(start16th) * m_16th.period },
                seconds{ static_cast<float>(current16th) * m_16th.period });
              foundNoteStart = false;
            }
          } else {
            if (c == '#') {
              // note stated
              start16th = current16th;
              foundNoteStart = true;
            }
          }

          current16th += 1;
        }
      }

      static constexpr auto extract_note_lines(std::string_view music) -> std::vector<std::string_view>
      {
        std::vector<std::string_view> noteLines{};

        constexpr auto END = std::string_view::npos;
        std::size_t pos{ 0 };

        do {
          auto i = music.find_first_of('\n', pos);
          if (i == END) {
            break;
          }

          auto len = i - pos;
          // check for \r\n
          if (i > 0 and music[i - 1] == '\r') {
            len -= 1;
          }
          noteLines.push_back(music.substr(pos, len));
          // move beyond the \n
          pos = i + 1;
        } while (pos < music.length());

        // filter empty lines and lines that start with spaces
        std::erase_if(noteLines, [](auto sv) { return sv.empty() or sv[0] == ' '; });

        // validate that all lines have a '|' at position 3 and last position and are same length
        if (noteLines.empty()) {
          throw std::invalid_argument{ "no music lines found" };
        }
        auto const len = noteLines[0].size();
        if (len < 6) {
          throw std::invalid_argument{ "music lines are too short" };
        }
        for (auto l : noteLines) {
          if (l.size() != len) {
            throw std::invalid_argument{ "music lines have different lengths" };
          }
          if (l[3] != '|') {
            throw std::invalid_argument{
              "music line must start with 2 or 3 char note name and have '|' in 4th position"
            };
          }
          if (!l.ends_with('|')) {
            throw std::invalid_argument{ "music line must end with '|'" };
          }
        }

        return noteLines;
      }
    };
  }  // namespace detail

  constexpr auto parse_music_length(auto getMusic) -> seconds
  {
    auto music = getMusic();
    detail::parser p{ music.bpm, music.source };
    return p.length();
  }

  template<sample_rate RATE, template<sample_rate> typename INSTRUMENT>
  class sequencer
  {
  public:
    constexpr explicit sequencer(INSTRUMENT<RATE> &instrument)
      : m_instrument{ instrument }
    {}

    constexpr void parse_music(auto getMusic)
    {
      auto music = getMusic();
      detail::parser p{ music.bpm, music.source };

      p.parse_events([&](note n, seconds on, seconds off) {
        m_eventQueueContainer.emplace_back(n, on.to_samples(RATE), off.to_samples(RATE));
      });

      std::make_heap(m_eventQueueContainer.begin(), m_eventQueueContainer.end(), event_min_sorter{});
    }

    constexpr void queue_event(note note, seconds noteOn, seconds noteOff)
    {
      queue_emplace(note, noteOn.to_samples(RATE), noteOff.to_samples(RATE));
    }

    constexpr void queue_event(note note, std::uint32_t noteOnSample, std::uint32_t noteOffSample)
    {
      queue_emplace(note, noteOnSample, noteOffSample);
    }

    template<block_size BLOCK_SIZE>
    constexpr void render(std::span<float, BLOCK_SIZE.samplesPerBlock> buffer)
    {
      // find events to trigger for this block
      std::uint32_t blockEndSampleNumber = m_blockStartSampleNumber + BLOCK_SIZE.samplesPerBlock - 1;
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
      m_blockStartSampleNumber += BLOCK_SIZE.samplesPerBlock;
    }

  private:
    // note events in absolute sample number time
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

    [[nodiscard]] constexpr auto queue_top() const -> event const &
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
    // but std::priority_queue is not constexpr
    std::vector<event> m_eventQueueContainer{};
  };
}  // namespace tmp