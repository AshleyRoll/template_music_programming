#include <array>

#include "../include/tmp/synth.hpp"
#include "../include/tmp/types.hpp"
#include "../include/tmp/wav_render.hpp"

int main(int argc, char *argv[])
{

  using namespace tmp;
  using namespace tmp::literals;

  wav_renderer_mono<sample_rate{ 4000 }, seconds{ 2.0F }> wav{};

  instruments::synth<wav.Rate> synth{};

  if(argc >= 1) {
    synth.play_note("C4"_note, 0.1_sec, 0.35_sec);
    synth.play_note("E4"_note, 0.5_sec, 0.4_sec);
    synth.play_note("G4"_note, 1.0_sec, 0.4_sec);
  }

  wav.render(synth);

  auto d = wav.data;
  return 0;
}