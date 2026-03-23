#include <iostream>
#include <iomanip>
#include <vector>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vprogmem.h"

class Testbench {
public:
    Vprogmem* top;
    VerilatedVcdC* tfp;
    uint64_t main_time;

    Testbench() {
        top = new Vprogmem;
        tfp = nullptr;
        main_time = 0;
        Verilated::traceEverOn(true);
    }

    ~Testbench() {
        if (tfp) {
            tfp->close();
            delete tfp;
        }
        delete top;
    }

    void opentrace(const char* filename) {
        tfp = new VerilatedVcdC;
        top->trace(tfp, 99);
        tfp->open(filename);
    }

    void tick() {
        top->clk = 0;
        top->eval();
        if (tfp) tfp->dump(main_time++);
        
        top->clk = 1;
        top->eval();
        if (tfp) tfp->dump(main_time++);
    }

    void reset() {
        top->_reset = 0;
        top->rx_empty = 1;
        top->r_data = 0;
        for (int i = 0; i < 10; i++) tick();
        top->_reset = 1;
        tick();
    }

    void send_byte(uint8_t byte) {
        top->r_data = byte;
        top->rx_empty = 0;

        int timeout = 0;
        while (!top->rd_uart && timeout < 100) {
            tick();
            timeout++;
        }

        tick();
        top->rx_empty = 1;
        
        for (int i = 0; i < 5; i++) tick();
    }
};

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Testbench tb;

    tb.opentrace("waveform.vcd");
    std::cout << "--- START SYMULACJI PROGRAMATORA 16-BIT ---" << std::endl;

    tb.reset();

    std::vector<uint8_t> hexdump = {
        0xAA, 0x11,
        0xBB, 0x22,
        0xCC, 0x33,
        0xDD, 0x44
    };

    for (size_t i = 0; i < hexdump.size(); i++) {
        std::cout << "Wysyłam bajt [" << i << "]: 0x" << std::hex << (int)hexdump[i] << std::endl;
        
        tb.send_byte(hexdump[i]);

        if (tb.top->prog2mem_write_en) {
            std::cout << ">>> ZAPIS DO PAMIĘCI!" << std::endl;
            std::cout << "    Adres: " << std::dec << (int)tb.top->prog2mem_addr << std::endl;
            std::cout << "    Dane (prog2mem_data): 0x" << std::hex << std::setw(4) << std::setfill('0') 
                      << (int)tb.top->prog2mem_data << std::endl;
            std::cout << "    Stream_en: " << (int)tb.top->prog2mem_stream_en << std::endl;
            std::cout << "---------------------------------------" << std::endl;
        }
    }

    for (int i = 0; i < 20; i++) tb.tick();

    std::cout << "Symulacja zakończona pomyślnie. Wyniki w waveform.vcd" << std::endl;
    return 0;
}