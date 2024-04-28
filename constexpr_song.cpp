#include <cstdint>
#include <array>


[[gnu::section(".wavefile"), gnu::used]]
constinit auto const WAVE_DATA = []{
    std::array<char, 100> data;
    data.fill('A');
    return data;
}();

