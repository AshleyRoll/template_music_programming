# Makefile for Compile Time Synthesizer

PROJ_NAME=constexpr_song

all: $(PROJ_NAME).wav

clean:
	@echo "Cleaning build outputs"
	@rm -rf obj/
	@rm -f $(PROJ_NAME).wav

$(PROJ_NAME).wav: obj/$(PROJ_NAME).elf
	@echo "Creating [ $@ ] from [ $< ] ..."
	@objcopy -O binary $< $@
	@echo "Done."

obj/$(PROJ_NAME).elf: src/$(PROJ_NAME).cpp
	@echo "Compiling [ $< ] ..."
	@mkdir -p obj
	@gcc -O3 --std=c++23 -c $< -T wav_gen.ld -o $@

