#include "progmem.h"

FlashMemmory16::FlashMemmory16() {
    memmory.fill(ERASED_VALUE);
}

FlashMemmory16::~FlashMemmory16() {
    // No cleanup necessary for std::array
}

uint16_t FlashMemmory16::read(uint16_t address) {
    if (address >= memmory.size()) return ERASED_VALUE;
    return memmory[address];
}

uint32_t FlashMemmory16::read32(uint16_t address) {
    if (address + 1 >= memmory.size())
        return ERASED_VALUE;

    return (static_cast<uint32_t>(memmory[address]) << 16) |
            static_cast<uint32_t>(memmory[address + 1]);
}

bool FlashMemmory16::write(uint16_t address, uint16_t data) {
    if (address >= memmory.size()) return false;
    return memmory[address] = data;
}

bool FlashMemmory16::program(std::string filename) {
    //Open code file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }

    std::string line;
    uint16_t pointer = 0;
    int count = 1;
    
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == '/') {
            continue;
        }
        
        // Remove whitespace
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        if (line[0] == 'x') {
            std::string loc = line.substr(1);

            try {
                pointer = std::stoul(loc, nullptr, 16);
                std::cout << "Pointer moved to 0x" << std::hex << pointer << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error parsing pointer: " << line << " - " << e.what() << std::endl;
                return false;
            }
            continue;
        }
        
        try {
            // Convert binary string to integer
            if (pointer >= memmory.size()) {
                std::cerr << "Error: Program too large for memory at " << pointer << std::endl;
                return false;
            }
            memmory[pointer++] = std::stoul(line, nullptr, 2);

            std::cout << "Loaded instruction " << count++ 
                        << ": 0x" << std::hex << memmory[pointer-1] << " (binary: " << line << ")" << std::dec 
                        << "; @ 0x" << std::hex << pointer-1 << std::dec << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing instruction: " << line << " - " << e.what() << std::endl;
            return false;
        }
    }

    return true;
}