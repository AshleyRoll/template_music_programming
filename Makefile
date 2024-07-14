# Makefile for Compile Time Synthesizer

SRC_FILES := $(shell find "src" -name "*.cpp")
HDR_FILES := $(shell find "include" -name "*.hpp")
OBJ_FILES := $(patsubst src/%.cpp, obj/%.o, $(SRC_FILES))
WAV_FILES := $(patsubst src/%.cpp, bin/%.wav, $(SRC_FILES))

CXX=g++
CPPFLAGS_GENERAL=--std=c++23 -I ./include -fconstexpr-ops-limit=9999999999999
CPPFLAGS=-O3 $(CPPFLAGS_GENERAL)
RM=rm -f
TIME=/usr/bin/time

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
	@$(TIME)  --format " --> [ $< ] %E elapsed" $(CXX) $(CPPFLAGS) -c $< -o $@


bin/%.wav: obj/%.o $(HDR_FILES)
	@echo "Extracting [ $@ ] from [ $< ]"
	@mkdir -p bin
	@objcopy --only-section=.wavefile -O binary $< $@


test: tests/test.cpp $(HDR_FILES)
	@echo "Building Runtime Test Executable"
	@mkdir -p bin
	@$(CXX) $(CPPFLAGS_GENERAL) -O0 -g $< -o bin/test
