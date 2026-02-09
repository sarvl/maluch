#ifndef PROGMEM_H
#define PROGMEM_H

#include <array>
#include <cstdint>
#include <string>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

class FlashMemmory16 {
    private:
        static constexpr size_t MEM_SIZE = 0x10000;
        std::array<uint16_t, MEM_SIZE> memmory;
        static constexpr uint16_t ERASED_VALUE = 0x0000;

    public:
        FlashMemmory16();
        ~FlashMemmory16();
        uint16_t read(uint16_t address);
        bool write(uint16_t address, uint16_t data);
        bool program(std::string file);
};

#endif