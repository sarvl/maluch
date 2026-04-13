#include <verilated.h>
#include <verilated_fst_c.h>
#include "Vuart.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vuart* dut = new Vuart;

    int num_runs = 1000;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.substr(0, 2) == "n=") {
            num_runs = std::stoi(arg.substr(2));
        }
    }

    Verilated::traceEverOn(true);
    VerilatedFstC* tfp = new VerilatedFstC;
    dut->trace(tfp, 99);
    tfp->open("waveform.fst");

    vluint64_t main_time = 0;

    auto cycle = [&]() {
        dut->clk = 1;
        dut->eval();
        tfp->dump(main_time++);
        dut->clk = 0;
        dut->eval();
        tfp->dump(main_time++);
    };

    dut->reset = 1;
    dut->rd_uart = 0;
    dut->wr_uart = 0;
    dut->rx = 1;
    dut->w_data = 0;

    for (int i = 0; i < 10; i++) cycle();
    dut->reset = 0;

    dut->w_data = 0xA5;
    dut->wr_uart = 1;
    cycle();
    dut->wr_uart = 0;

    for (int i = 0; i < num_runs; i++) {
        dut->rx = dut->tx;
        cycle();
    }

    tfp->close();
    delete dut;
    return 0;
}