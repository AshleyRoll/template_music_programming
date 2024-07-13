# Makefile for Compile Time Synthesizer

SRC_FILES := src/song.cpp src/test.cpp
HDR_FILES := $(shell find include -name "*.hpp")
OBJ_FILES := $(patsubst src/%.cpp, obj/%.o, $(SRC_FILES))
WAV_FILES := $(patsubst src/%.cpp, bin/%.wav, $(SRC_FILES))

CXX=g++
CPPFLAGS=-O3 --std=c++23 -I ./include -fconstexpr-ops-limit=9999999999999
LDFLAGS=-T $(LD_SCRIPT)
LD=ld
RM=rm -f

.PHONY: all clean
.PRECIOUS: $(OBJ_FILES)

all: $(WAV_FILES)

play-%: bin/%.wav
	@echo "Playing [ $< ]"
	@play $<

clean:
	@echo "Cleaning build outputs"
	@$(RM) -r obj
	@$(RM) -r bin


obj/%.o: src/%.cpp $(HDR_FILES)
	@echo "Compiling [ $< ]"
	@mkdir -p obj
	@$(CXX) $(CPPFLAGS) -c $< -o $@


bin/%.wav: obj/%.o $(HDR_FILES)
	@echo "Extracting [ $@ ] from [ $< ]"
	@mkdir -p bin
	@objcopy --only-section=.wavefile -O binary $< $@

