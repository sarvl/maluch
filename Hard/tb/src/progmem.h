#ifndef PROGMEM_H
#define PROGMEM_H

#include <vector>
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
        std::vector<uint16_t> memmory;
        static constexpr uint16_t ERASED_VALUE = 0x0000;
        
        size_t mem_size;

    public:
        FlashMemmory16(size_t size = 0x10000);
        ~FlashMemmory16();
        uint16_t read(uint16_t address);
        bool write(uint16_t address, uint16_t data);
        bool program(std::string file);
};

#endif