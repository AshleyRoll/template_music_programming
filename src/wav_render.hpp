#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <span>

namespace tmp {

    struct sample_rate {
        std::uint32_t samples_per_second;
    };
    struct seconds {
        std::uint32_t seconds;
    };

    namespace detail {
        constexpr static void write_le(std::span<std::byte> buffer, std::uint16_t data) {
            buffer[0] = static_cast<std::byte>(data & 0xFF);
            buffer[1] = static_cast<std::byte>(static_cast<std::uint16_t>(data >> 8U) & 0xFFU);
        }

        constexpr static void write_le(std::span<std::byte> buffer, std::uint32_t data) {
            buffer[0] = static_cast<std::byte>(data & 0xFF);
            buffer[1] = static_cast<std::byte>(static_cast<std::uint32_t>(data >> 8U) & 0xFFU);
            buffer[2] = static_cast<std::byte>(static_cast<std::uint32_t>(data >> 16U) & 0xFFU);
            buffer[3] = static_cast<std::byte>(static_cast<std::uint32_t>(data >> 24U) & 0xFFU);
        }

        constexpr static void write_be(std::span<std::byte> buffer, std::uint16_t data) {
            buffer[1] = static_cast<std::byte>(data & 0xFF);
            buffer[0] = static_cast<std::byte>(static_cast<std::uint16_t>(data >> 8U) & 0xFFU);
        }

        constexpr static void write_be(std::span<std::byte> buffer, std::uint32_t data) {
            buffer[3] = static_cast<std::byte>(data & 0xFF);
            buffer[2] = static_cast<std::byte>(static_cast<std::uint32_t>(data >> 8U) & 0xFFU);
            buffer[1] = static_cast<std::byte>(static_cast<std::uint32_t>(data >> 16U) & 0xFFU);
            buffer[0] = static_cast<std::byte>(static_cast<std::uint32_t>(data >> 24U) & 0xFFU);
        }

        template<sample_rate RATE>
        struct wav_fmt_chunk {
            constexpr static std::uint32_t Size = 16 + 4 + 4;
            constexpr static std::uint8_t BytesPerSample = 2;   // forcing 16 bit

            constexpr static std::uint32_t SubChunkId = 0x666d7420; // "fmt ", big endian
            constexpr static std::uint32_t SubChunkSize = 16;       // bytes after this entry
            constexpr static std::uint16_t AudioFormat = 0x0001;    // PCM
            constexpr static std::uint16_t NumberChannels = 0x0001; // 1 channel / mono
            constexpr static std::uint32_t SampleRate = RATE.samples_per_second;
            constexpr static std::uint32_t ByteRate = SampleRate * NumberChannels * BytesPerSample;
            constexpr static std::uint16_t BlockAlign = NumberChannels * BytesPerSample;
            constexpr static std::uint16_t BitsPerSample = BytesPerSample * 8;      // 16 bit PCM

            constexpr static auto number_samples(seconds seconds) -> std::uint32_t {
                return seconds.seconds * SampleRate * NumberChannels;
            }

            constexpr static auto sample_data_length(seconds seconds) -> std::uint32_t {
                return number_samples(seconds) * BytesPerSample;
            }

            constexpr static void render(std::span<std::byte, Size> buffer) {
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


        struct wav_data_chunk_header {
            constexpr static std::uint32_t Size = 8;    // followed by data
            constexpr static std::uint32_t SubChunkId = 0x64617461; // "data", big endian

            constexpr static void render(std::span<std::byte, Size> buffer, std::uint32_t sampleDataLength) {
                detail::write_be(buffer.subspan<0, 4>(), SubChunkId);
                detail::write_le(buffer.subspan<4, 4>(), sampleDataLength);
                // NOTE: next comes the sample data which will be rendered separately.
            }
        };

        template<sample_rate RATE>
        struct riff_header {
            constexpr static std::uint32_t Size = 4 * 3;
            constexpr static std::uint32_t ChunkId = 0x52494646;    // "RIFF", big endian
            constexpr static std::uint32_t Format = 0x57415645;     // "WAVE", bit endian

            constexpr static void render(std::span<std::byte, Size> buffer, std::uint32_t sampleDataLength) {
                std::uint32_t chunkSize = 4 + wav_fmt_chunk<RATE>::Size + wav_data_chunk_header::Size + sampleDataLength;

                detail::write_be(buffer.subspan<0, 4>(), ChunkId);
                detail::write_le(buffer.subspan<4, 4>(), chunkSize);
                detail::write_be(buffer.subspan<8, 4>(), Format);
            }
        };

    }


    template<sample_rate RATE, seconds SECONDS>
    struct wav_renderer_mono {
        using RiffHdr = detail::riff_header<RATE>;
        using Fmt = detail::wav_fmt_chunk<RATE>;
        using WavHdr = detail::wav_data_chunk_header;

        static constexpr std::uint32_t NumSamples = Fmt::number_samples(SECONDS);
        static constexpr std::uint32_t SampleDataLength = Fmt::sample_data_length(SECONDS);

        static constexpr std::size_t TotalSize = RiffHdr::Size + Fmt::Size + WavHdr::Size + SampleDataLength;

        std::array<std::byte, TotalSize> data;

        constexpr void render() {
            std::span < std::byte, TotalSize > buffer{data};

            RiffHdr::render(buffer.template subspan<0, RiffHdr::Size>(), SampleDataLength);
            Fmt::render(buffer.template subspan<RiffHdr::Size, Fmt::Size>());
            WavHdr::render(buffer.template subspan<RiffHdr::Size + Fmt::Size, WavHdr::Size>(), SampleDataLength);

            auto sampleData = buffer.template last<SampleDataLength>();

            auto const frequency = 440.0;
            auto const thetaIncrement = (2.0 * std::numbers::pi * frequency) / RATE.samples_per_second;

            auto theta = 0.0;
            for(std::size_t sample = 0; sample < NumSamples; ++sample) {
                auto s = static_cast<std::int16_t>(std::sin(theta) * std::numeric_limits<int16_t>::max());
                detail::write_le(sampleData.subspan(sample*2, 2), static_cast<std::uint16_t>(s));

                theta += thetaIncrement;
            }
        }
    };
}