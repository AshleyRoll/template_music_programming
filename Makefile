# Makefile for Compile Time Synthesizer

PROJ_NAME=constexpr_song

all: $(PROJ_NAME).wav

clean:
	@echo "Cleaning build outputs"
	@rm -rf obj/
	@rm -f $(PROJ_NAME).wav

$(PROJ_NAME).wav: obj/$(PROJ_NAME).elf
	@echo "Creating WAV file from ELF"
	@objcopy -O binary $< $(PROJ_NAME).wav

obj/$(PROJ_NAME).elf: $(PROJ_NAME).cpp
	@echo "Compiling $<"
	@mkdir -p obj
	@gcc -O3 --std=c++23 -c $< -T wav_gen.ld -o obj/$(PROJ_NAME).elf

