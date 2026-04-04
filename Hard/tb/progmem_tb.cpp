#include <iostream>
#include <vector>
#include <iomanip>
#include <cstring>
#include <verilated.h>
#include <verilated_fst_c.h>
#include "Vprogmem.h"

int main(int argc, char** argv) {
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    contextp->commandArgs(argc, argv);

    int runs = 1000;
    const char* arg_n = contextp->commandArgsPlusMatch("n=");
    if (arg_n && strlen(arg_n) > 2) {
        runs = atoi(arg_n + 2);
    }

    Vprogmem* top = new Vprogmem{contextp.get()};
    Verilated::traceEverOn(true);
    VerilatedFstC* tfp = new VerilatedFstC;
    top->trace(tfp, 99);
    tfp->open("waveform.fst");

    int main_time = 0;

    top->_reset = 0;
    top->clk = 0;
    top->rx_empty = 1;
    top->eval();
    tfp->dump(main_time++);
    
    top->clk = 1;
    top->eval();
    tfp->dump(main_time++);
    top->_reset = 1;

    std::cout << "[TB] Start symulacji (max cykli: " << runs << ")" << std::endl;

    std::vector<uint8_t> test_data = {0x34, 0x12};
    size_t byte_idx = 0;

    for (int i = 0; i < runs; i++) {
        top->clk = !top->clk;
        
        if (top->clk == 1 && byte_idx < test_data.size()) {
            top->r_data = test_data[byte_idx];
            top->rx_empty = 0;
            
            if (top->rd_uart) {
                byte_idx++;
            }
        } else if (byte_idx >= test_data.size()) {
            top->rx_empty = 1;
        }

        top->eval();
        tfp->dump(main_time++);

        if (top->prog2mem_write_en && top->clk) { 
            std::cout << ">>> Zapisano: 0x" << std::hex << (int)top->prog2mem_data 
                      << " pod adres: 0x" << (int)top->prog2mem_addr << std::endl;
            if (byte_idx >= test_data.size()) break;
        }
    }

    tfp->close();
    delete tfp;
    delete top;
    return 0;
}