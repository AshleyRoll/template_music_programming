#pragma once

#include <cmath>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace tmp {

  struct sample_rate
  {
    std::uint32_t samples_per_second;
  };

  struct seconds
  {
    float seconds;

    [[nodiscard]] constexpr auto to_samples(sample_rate rate) const -> std::uint32_t
    {
      return static_cast<std::uint32_t>(seconds * static_cast<float>(rate.samples_per_second));
    }
  };

  struct frequency
  {
    float hertz;
  };

  struct volume
  {
    constexpr explicit volume(float value)
      : value{ std::clamp(value, 0.0F, 1.0F) }
    {}

    float value;
  };

  struct block_size
  {
    std::size_t samples_per_block;
  };

  struct note
  {
    constexpr explicit note(std::string_view const name)
      : note_frequency{ parse(name) }
    {}

    frequency note_frequency;

  private:
    constexpr static auto parse(std::string_view note) -> frequency
    {
      // Name in format of <N><Octave>
      // Where name is in list below, and Octave is 0-8.
      //
      // deal with extended piano keys. key 1 = A0, key 88 = C8, key 99 = B8 - up to octave 8
      // where A4 == 440Hz. We use the offset into notes (+1) as the note number and calculate
      // the frequency from formula here: https://en.wikipedia.org/wiki/Piano_key_frequencies

      constexpr static std::array<std::string_view, 12> Notes{
        "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"
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
      auto noteNumber = static_cast<int>(1 + std::distance(Notes.begin(), pos));

      // find the octave number
      if (octave[0] < '0' || octave[0] > '8') {
        throw std::invalid_argument{ "Invalid octave number (0..8)" };
      }
      auto octaveNumber = static_cast<int>(octave[0] - '0');
      noteNumber = noteNumber + (12 * octaveNumber);

      // compute the frequency
      return frequency{ std::pow(2.0F, ((static_cast<float>(noteNumber) - 49) / 12.0F)) * 440.0F };
    }
  };

}  // namespace tmp