cmake_minimum_required(VERSION 3.22)
project(TemplateMusicProgramming VERSION 0.1.0 LANGUAGES CXX)

macro(add_wav TARGET SOURCES)
    add_library(${TARGET}-obj OBJECT ${SOURCES})
    target_include_directories(${TARGET}-obj PUBLIC include)
    target_compile_features(${TARGET}-obj PUBLIC cxx_std_23)
    target_compile_options(${TARGET}-obj PRIVATE -fconstexpr-ops-limit=9999999999999)

    # add additional target to extract the WAV data into a separate file
    add_custom_target(
        ${TARGET}
        DEPENDS ${TARGET}-obj
        COMMAND ${CMAKE_OBJCOPY} --only-section=.wavefile -O binary $<TARGET_OBJECTS:${TARGET}-obj> ${TARGET}.wav
        BYPRODUCTS ${TARGET}.wav
        VERBATIM
    )
endmacro()

# Add songs here, build this target name to generate the WAV files.
add_wav(song src/song.cpp)
add_wav(simple src/simple.cpp)

# add our test target for run-time debugging
add_executable(runtime-test tests/test.cpp)
target_include_directories(runtime-test PRIVATE include)
target_compile_features(runtime-test PUBLIC cxx_std_23)
