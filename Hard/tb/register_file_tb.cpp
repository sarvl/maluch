#include "Vregister_file.h"
#include "verilated.h"
#include "verilated_fst_c.h"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>

using namespace std;

VerilatedContext* contextp = nullptr;
Vregister_file* top = nullptr;
VerilatedFstC* tfp = nullptr;

vluint64_t main_time = 0;
double sc_time_stamp() { return main_time; }

// Utility functions
void tick() {
    top->clk = 0;
    top->eval();
    if (tfp) tfp->dump(contextp->time());
    contextp->timeInc(1);
    main_time++;

    top->clk = 1;
    top->eval();
    if (tfp) tfp->dump(contextp->time());
    contextp->timeInc(1);
    main_time++;
}

void apply_reset() {
    top->_reset = 1;
    tick();
    tick();
    top->_reset = 0;
    tick();
}

// Test functions
void test_write_read(int addr, uint16_t value) {
    top->reg_w_en = 1;
    top->addr_in = addr;
    top->reg_in = value;
    tick();
    top->reg_w_en = 0;

    top->addr_out1 = addr;
    tick();
    uint16_t read_value = top->reg_out1;

    if (read_value == value) {
        cout << "[PASS] Addr " << addr << ": Wrote " << value << ", Read " << read_value << endl;
    } else {
        cout << "[FAIL] Addr " << addr << ": Wrote " << value << ", Read " << read_value << endl;
    }
}

int main(int argc, char** argv) {
    contextp = new VerilatedContext;
    contextp->debug(0);
    contextp->randSeed(0);

    top = new Vregister_file(contextp);

    Verilated::traceEverOn(true);
    tfp = new VerilatedFstC;
    top->trace(tfp, 99);
    system("mkdir -p waveforms >/dev/null 2>&1");
    tfp->open("waveforms/register_file.fst");

    // Initialize signals
    top->clk = 0;
    top->_reset = 0;
    top->reg_w_en = 0;
    top->addr_in = 0;
    top->reg_in = 0;
    top->addr_out1 = 0;
    top->addr_out2 = 0;

    // Apply reset
    apply_reset();

    // Test write and read
    test_write_read(1, 0x1234);
    test_write_read(2, 0xABCD);
    test_write_read(3, 0x5678);

    // Finish simulation
    tfp->close();
    top->final();

    delete tfp;
    delete top;
    delete contextp;

    return 0;
}
