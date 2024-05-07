# Makefile for Compile Time Synthesizer

PROJ_NAME := constexpr_song

SRC_FILES := src/constexpr_song.cpp
OBJ_FILES := $(patsubst src/%.cpp, obj/%.o, $(SRC_FILES))

LD_SCRIPT := gcc/wav_file_generator.ld

CXX=g++
CPPFLAGS=-O3 --std=c++23 -I ./include -fconstexpr-ops-limit=9999999999999
LDFLAGS=-T $(LD_SCRIPT)
LD=ld
RM=rm -f

all: $(PROJ_NAME).wav

play: all
	@play $(PROJ_NAME).wav

clean:
	@echo "Cleaning build outputs"
	@$(RM) -r obj
	@$(RM) $(PROJ_NAME).wav

obj/%.o: src/%.cpp
	@echo "Compiling [ $^ ]"
	@mkdir -p obj
	@$(CXX) $(CPPFLAGS) -c $< -o $@

$(PROJ_NAME).wav: obj/$(PROJ_NAME).elf
	@echo "Extracting [ $@ ] from [ $< ] ..."
	@objcopy --only-section=.wavefile -O binary $< $@
	@echo "Done."


obj/$(PROJ_NAME).elf: $(OBJ_FILES) $(LD_SCRIPT)
	@echo "Linking [ $@ ] ..."
	@mkdir -p obj
	@$(LD) $(LDFLAGS) -o $@ $(OBJ_FILES)

depend: .depend

.depend: $(SRC_FILES)
	@echo "Computing Dependencies..."
	@rm -f "$@"
	@$(CXX) $(CPPFLAGS) -MM $^ -MF "$@"

-include .depend
