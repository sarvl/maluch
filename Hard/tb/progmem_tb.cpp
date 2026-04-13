#include <iostream>
#include <vector>
#include <queue>
#include <iomanip>
#include <memory>
#include <verilated.h>
#include <verilated_fst_c.h>
#include "Vprogmem.h"

vluint64_t sim_time = 0;

double sc_time_stamp() { return sim_time; }

void tick(Vprogmem* top, VerilatedFstC* tfp) {
    top->clk = 0;
    top->eval();
    tfp->dump(sim_time++);

    top->clk = 1;
    top->eval();
    tfp->dump(sim_time++);
}

int main(int argc, char** argv) {
    auto contextp = std::make_unique<VerilatedContext>();
    contextp->commandArgs(argc, argv);

    auto top = std::make_unique<Vprogmem>(contextp.get());

    Verilated::traceEverOn(true);
    auto tfp = std::make_unique<VerilatedFstC>();
    top->trace(tfp.get(), 99);
    tfp->open("waveform.fst");

    top->_reset = 0;
    top->rx_empty = 1;
    tick(top.get(), tfp.get());
    tick(top.get(), tfp.get());
    top->_reset = 1;

    std::cout << "[TB] Start\n";

    std::vector<uint8_t> input_bytes = {
        0x34, 0x12,
        0x78, 0x56,
        0xBC, 0x9A
    };

    std::vector<uint16_t> expected = {
        0x1234,
        0x5678,
        0x9ABC
    };

    std::queue<uint8_t> fifo;

    for (auto b : input_bytes) fifo.push(b);

    int expected_idx = 0;
    int max_cycles = 200;

    for (int cycle = 0; cycle < max_cycles; cycle++) {

        if (!fifo.empty()) {
            top->rx_empty = 0;
            top->r_data = fifo.front();
        } else {
            top->rx_empty = 1;
        }

        if (top->rd_uart && !fifo.empty()) {
            fifo.pop();
        }

        tick(top.get(), tfp.get());

        if (top->prog2mem_write_en) {
            uint16_t got = top->prog2mem_data;
            uint16_t exp = expected[expected_idx];

            std::cout << "[WRITE] addr=" << std::dec << top->prog2mem_addr
                      << " data=0x" << std::hex << std::setw(4) << std::setfill('0') << got;

            if (got == exp) {
                std::cout << "  OK\n";
            } else {
                std::cout << "  ERROR (expected 0x" << std::setw(4) << exp << ")\n";
                return 1;
            }

            expected_idx++;
            if (expected_idx == expected.size()) break;
        }
    }

    if (expected_idx != expected.size()) {
        std::cout << "[TB] ERROR: nie wszystkie dane zapisane\n";
        return 1;
    }

    std::cout << "[TB] PASS\n";

    tfp->close();
    return 0;
}