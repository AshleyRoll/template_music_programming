#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <span>

#include "synth.hpp"

namespace tmp {

  namespace detail {
    constexpr static void write_le(std::span<std::byte> buffer, std::uint16_t data)
    {
      buffer[0] = static_cast<std::byte>(data & 0xFF);
      buffer[1] = static_cast<std::byte>(static_cast<std::uint16_t>(data >> 8U) & 0xFFU);
    }

    constexpr static void write_le(std::span<std::byte> buffer, std::uint32_t data)
    {
      buffer[0] = static_cast<std::byte>(data & 0xFF);
      buffer[1] = static_cast<std::byte>(static_cast<std::uint32_t>(data >> 8U) & 0xFFU);
      buffer[2] = static_cast<std::byte>(static_cast<std::uint32_t>(data >> 16U) & 0xFFU);
      buffer[3] = static_cast<std::byte>(static_cast<std::uint32_t>(data >> 24U) & 0xFFU);
    }

    constexpr static void write_be(std::span<std::byte> buffer, std::uint16_t data)
    {
      buffer[1] = static_cast<std::byte>(data & 0xFF);
      buffer[0] = static_cast<std::byte>(static_cast<std::uint16_t>(data >> 8U) & 0xFFU);
    }

    constexpr static void write_be(std::span<std::byte> buffer, std::uint32_t data)
    {
      buffer[3] = static_cast<std::byte>(data & 0xFF);
      buffer[2] = static_cast<std::byte>(static_cast<std::uint32_t>(data >> 8U) & 0xFFU);
      buffer[1] = static_cast<std::byte>(static_cast<std::uint32_t>(data >> 16U) & 0xFFU);
      buffer[0] = static_cast<std::byte>(static_cast<std::uint32_t>(data >> 24U) & 0xFFU);
    }

    template<sample_rate RATE>
    struct wav_fmt_chunk
    {
      constexpr static std::uint32_t Size = 16 + 4 + 4;
      constexpr static std::uint8_t BytesPerSample = 2;  // forcing 16 bit

      constexpr static std::uint32_t SubChunkId = 0x666d7420;  // "fmt ", big endian
      constexpr static std::uint32_t SubChunkSize = 16;  // bytes after this entry
      constexpr static std::uint16_t AudioFormat = 0x0001;  // PCM
      constexpr static std::uint16_t NumberChannels = 0x0001;  // 1 channel / mono
      constexpr static std::uint32_t SampleRate = RATE.samples_per_second;
      constexpr static std::uint32_t ByteRate = SampleRate * NumberChannels * BytesPerSample;
      constexpr static std::uint16_t BlockAlign = NumberChannels * BytesPerSample;
      constexpr static std::uint16_t BitsPerSample = BytesPerSample * 8;  // 16 bit PCM

      constexpr static auto number_samples(seconds seconds, block_size blockSize) -> std::uint32_t
      {
        // ensure we end up with a whole number of blocks
        auto samples = seconds.seconds * SampleRate * NumberChannels;
        return ((samples / blockSize.samples_per_block) + 1) * blockSize.samples_per_block;
      }

      constexpr static auto sample_data_length(seconds seconds, block_size blockSize) -> std::uint32_t
      {
        return number_samples(seconds, blockSize) * BytesPerSample;
      }

      constexpr static void render(std::span<std::byte, Size> buffer)
      {
        detail::write_be(buffer.subspan<0, 4>(), SubChunkId);
        detail::write_le(buffer.subspan<4, 4>(), SubChunkSize);
        detail::write_le(buffer.subspan<8, 2>(), AudioFormat);
        detail::write_le(buffer.subspan<10, 2>(), NumberChannels);
        detail::write_le(buffer.subspan<12, 4>(), SampleRate);
        detail::write_le(buffer.subspan<16, 4>(), ByteRate);
        detail::write_le(buffer.subspan<20, 2>(), BlockAlign);
        detail::write_le(buffer.subspan<22, 2>(), BitsPerSample);
      }
    };


    struct wav_data_chunk_header
    {
      constexpr static std::uint32_t Size = 8;  // followed by data
      constexpr static std::uint32_t SubChunkId = 0x64617461;  // "data", big endian

      constexpr static void render(std::span<std::byte, Size> buffer, std::uint32_t sampleDataLength)
      {
        detail::write_be(buffer.subspan<0, 4>(), SubChunkId);
        detail::write_le(buffer.subspan<4, 4>(), sampleDataLength);
        // NOTE: next comes the sample data which will be rendered separately.
      }
    };

    template<sample_rate RATE>
    struct riff_header
    {
      constexpr static std::uint32_t Size = 4 * 3;
      constexpr static std::uint32_t ChunkId = 0x52494646;  // "RIFF", big endian
      constexpr static std::uint32_t Format = 0x57415645;  // "WAVE", bit endian

      constexpr static void render(std::span<std::byte, Size> buffer, std::uint32_t sampleDataLength)
      {
        std::uint32_t chunkSize = 4 + wav_fmt_chunk<RATE>::Size + wav_data_chunk_header::Size + sampleDataLength;

        detail::write_be(buffer.subspan<0, 4>(), ChunkId);
        detail::write_le(buffer.subspan<4, 4>(), chunkSize);
        detail::write_be(buffer.subspan<8, 4>(), Format);
      }
    };

  }  // namespace detail


  template<sample_rate RATE, seconds SECONDS, block_size BLOCK_SIZE>
  struct wav_renderer_mono
  {
    static constexpr sample_rate Rate = RATE;
    using RiffHdr = detail::riff_header<RATE>;
    using Fmt = detail::wav_fmt_chunk<RATE>;
    using WavHdr = detail::wav_data_chunk_header;

    static constexpr std::uint32_t NumSamples = Fmt::number_samples(SECONDS, BLOCK_SIZE);
    static constexpr std::uint32_t SampleDataLength = Fmt::sample_data_length(SECONDS, BLOCK_SIZE);

    static constexpr std::size_t TotalSize = RiffHdr::Size + Fmt::Size + WavHdr::Size + SampleDataLength;

    std::array<std::byte, TotalSize> data;

    template<typename SOURCE>
    constexpr void render(SOURCE source)
    {
      std::span<std::byte, TotalSize> buffer{ data };

      RiffHdr::render(buffer.template subspan<0, RiffHdr::Size>(), SampleDataLength);
      Fmt::render(buffer.template subspan<RiffHdr::Size, Fmt::Size>());
      WavHdr::render(buffer.template subspan<RiffHdr::Size + Fmt::Size, WavHdr::Size>(), SampleDataLength);

      auto sampleData = buffer.template last<SampleDataLength>();
      std::array<float, BLOCK_SIZE.samples_per_block> audioData{};

      // We know we have a full multiple of BLOCK_SIZE blocks because of our calculations in the format helper
      for (std::size_t block{ 0 }; block < NumSamples; block += BLOCK_SIZE.samples_per_block) {
        source.template render<BLOCK_SIZE>(audioData);

        for(std::size_t sample{0}; sample < BLOCK_SIZE.samples_per_block; ++sample) {
          // encode sample to 16 bit
          auto clamped = std::clamp(audioData[sample], -1.0F, 1.0F);
          auto val = static_cast<std::int16_t>(clamped * std::numeric_limits<int16_t>::max());
          detail::write_le(sampleData.subspan((block + sample) * 2, 2), static_cast<std::uint16_t>(val));
        }

      }
    }
  };
}  // namespace tmp