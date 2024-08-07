#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace tmp {

  struct sample_rate
  {
    constexpr explicit sample_rate(std::uint32_t sps)
      : samples_per_second{ sps }
    {}

    std::uint32_t samples_per_second;
  };

  struct beats_per_minute
  {
    constexpr explicit beats_per_minute(std::uint16_t bpm)
      : rate{ bpm }
    {}

    std::uint16_t rate;
  };

  struct seconds
  {
    constexpr explicit seconds(float s)  // NOLINT
      : period{ s }
    {}

    float period;

    [[nodiscard]] constexpr auto to_samples(sample_rate rate) const -> std::uint32_t
    {
      return static_cast<std::uint32_t>(period * static_cast<float>(rate.samples_per_second));
    }
  };

  struct frequency
  {
    constexpr explicit frequency(float hz)  // NOLINT
      : hertz{ hz }
    {}

    float hertz;

    [[nodiscard]] constexpr auto to_period() const -> seconds
    {
      return seconds{ (1.0f / hertz) };
    }

    [[nodiscard]] constexpr auto to_half_period() const -> seconds
    {
      return seconds{ 1.0f / (2.0f * hertz) };
    }
  };

  struct dBfs
  {
    constexpr explicit dBfs(float db)  // NOLINT
      : value{ db }
    {}

    float value;

    [[nodiscard]] constexpr auto to_linear(float fsReference) const -> float
    {
      // power 10 * log(P/ref)
      // P = 10^(dB/10) * ref
      return std::pow(10.0F, value / 10.0F) * fsReference;
    }

    constexpr auto operator-() const -> dBfs
    {
      return dBfs{ -value };
    }
  };

  struct volume
  {
    constexpr static float Min = 0.0F;
    constexpr static float Max = 1.0F;

    constexpr explicit volume(float linearVolume)
      : value{ linearVolume }
    {
      value = std::clamp(value, Min, Max);
    }

    constexpr volume(dBfs dbfs)
      : volume(dbfs.to_linear(Max))
    {}


    float value;
  };

  struct block_size
  {
    constexpr explicit block_size(std::uint32_t samplesPerBlock)
      : samplesPerBlock{ samplesPerBlock }
    {}

    std::uint32_t samplesPerBlock;
  };

  struct music
  {
    constexpr music(beats_per_minute bpm, std::string_view source)
      : bpm{ bpm }
      , source{ source }
    {}

    beats_per_minute bpm;
    std::string_view source;
  };


  struct note
  {
    // In MIDI, A0 is note 21, making A4 = 69, we will use MIDI note numbers
    // to make our life easier in converting data and making nodes from our piano roll
    constexpr static int A0NoteNumber = 21;
    constexpr static int A4NoteNumber = A0NoteNumber + (12 * 4);
    constexpr static float A4NoteFrequency = 440.0F;
    constexpr static int C0NoteNumber = 12;

    constexpr explicit note(std::string_view const name)
      : note_name{ name }
      , note_number{ parse_note_number(note_name) }
      , note_frequency{ calculate_frequency(note_number) }

    {}

    std::string_view note_name;
    int note_number;
    frequency note_frequency;

  private:
    constexpr static auto parse_note_number(std::string_view note) -> int
    {
      // Name in format of <N><Octave>
      // Where name is in list below, and Octave is 0-8.
      // We use the offset into music as the note number and calculate
      // the frequency from formula here: https://en.wikipedia.org/wiki/Piano_key_frequencies

      // octave numbers start at C
      constexpr static std::array<std::string_view, 12> Notes{
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
      };

      std::string_view name;
      std::string_view octave;
      if (note.length() == 2) {
        name = note.substr(0, 1);
        octave = note.substr(1, 1);
      } else if (note.length() == 3) {
        name = note.substr(0, 2);
        octave = note.substr(2, 1);
      } else {
        throw std::invalid_argument{ "Invalid note name length" };
      }

      // find note name in list and get its index
      auto pos = std::find(Notes.begin(), Notes.end(), name);
      if (pos == Notes.end()) {
        throw std::invalid_argument{ "Invalid note name, (A-G#, no B# or E#)" };
      }
      auto noteNumber = static_cast<int>(C0NoteNumber + std::distance(Notes.begin(), pos));

      // find the octave number
      if (octave[0] < '0' || octave[0] > '8') {
        throw std::invalid_argument{ "Invalid octave number (0..8)" };
      }
      auto octaveNumber = static_cast<int>(octave[0] - '0');

      return noteNumber + (12 * octaveNumber);
    }

    constexpr static auto calculate_frequency(int noteNumber) -> frequency
    {
      // compute the frequency
      return frequency{ std::pow(2.0F, (static_cast<float>(noteNumber - A4NoteNumber) / 12.0F)) * A4NoteFrequency };
    }
  };

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
  struct envelope
  {
    constexpr envelope(seconds attackTime,
      volume attackLevel,
      seconds decayTime,
      volume decayLevel,
      seconds releaseTime)
      : attackTime{ attackTime }
      , attackLevel{ attackLevel }
      , decayTime{ decayTime }
      , decayLevel{ decayLevel }
      , releaseTime{ releaseTime }
    {}

    seconds attackTime;
    volume attackLevel;
    seconds decayTime;
    volume decayLevel;
    seconds releaseTime;
  };


  namespace literals {
    constexpr auto operator""_dBfs(long double value) -> dBfs
    {
      return dBfs{ static_cast<float>(value) };
    }

    constexpr auto operator""_sec(long double value) -> seconds
    {
      return seconds{ static_cast<float>(value) };
    }

    constexpr auto operator""_hz(long double value) -> frequency
    {
      return frequency{ static_cast<float>(value) };
    }

    constexpr auto operator""_note(char const *str, std::size_t size) -> note
    {
      return note{
        std::string_view{ str, size }
      };
    }
  }  // namespace literals

}  // namespace tmp