#include "Vmemory.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

class MemoryTestBench {
private:
    bool clk = false;
    uint64_t cycle_count = 0;
    
public:
    uint64_t ns_count = 0;
    bool toggleClock() {
        ns_count = (ns_count + 1) % 20;
        bool next_clk = (ns_count <= 10);

        if (!clk && next_clk) {
            cycle_count++;
        }
        clk = next_clk;
        return clk;
    }

    uint64_t getCycleCount() const {
        return cycle_count;
    }

    void reset() {
        clk = false;
        cycle_count = 0;
        ns_count = 0;
    }


};

int main(int argc, char* argv[]) {
    uint64_t max_cycles = 200;
    bool enable_trace = true;
    bool verbose = true;

    Verilated::commandArgs(argc, argv);
    VerilatedContext* contextp = new VerilatedContext;
    Vmemory* mem = new Vmemory(contextp);

    VerilatedFstC* tfp = nullptr;
    if (enable_trace) {
        Verilated::traceEverOn(true);
        tfp = new VerilatedFstC;
        mem->trace(tfp, 99);
        std::filesystem::create_directories("waveforms");
        tfp->open("waveforms/memory.fst");
        std::cout << "Waveform tracing enabled: memory.fst" << std::endl;
    }

    MemoryTestBench tb;
    tb.reset();

    std::cout << "\n=== Starting Memory Simulation ===" << std::endl;

    // Initial evaluation
    mem->clk = 0;
    mem->write_en = 0;
    mem->addr = 0;
    mem->instr_addr = 0;          // new port
    mem->data_in = 0;
    mem->mem_vram_addr = 0;
    mem->mem_prog_write_en = 0;
    mem->eval();
    if (tfp) tfp->dump(contextp->time());
    contextp->timeInc(1);

    // Test variables
    uint16_t ram_addr = 0x8000;
    uint16_t ram_data = 0xABCD;
    uint16_t vram_addr = 0x4000;
    uint16_t vram_data = 0x1234;
    uint16_t rom_addr = 0x0000;
    uint16_t rom_data1 = 0x5678;
    uint16_t rom_data2 = 0x9ABC;

    // Flags for tests
    bool test_failed = false;
    bool ram_written = false;
    bool ram_read = false;
    bool vram_written = false;
    bool vram_read = false;
    bool rom_written1 = false;
    bool rom_written2 = false;
    bool rom_read = false;
    bool ram2_written = false;
    bool ram2_read = false;
    bool ram_instr_read = false;
    bool rom_data_read2 = false;
    bool rom_instr_read2 = false;
    bool rom_data_read_odd = false;
    bool ram_odd_written = false;
    bool ram_odd_read = false;

    // Main simulation loop
    while (tb.getCycleCount() < max_cycles) {
        mem->clk = tb.toggleClock();

        auto check = [&](const char* name, uint32_t got, uint32_t exp) {
            if (got != exp) {
                std::cerr << name << " failed: got 0x" << std::hex << std::setw(8) << std::setfill('0') << got
                          << " exp 0x" << std::setw(8) << exp << std::dec << std::endl;
                test_failed = true;
            } else {
                std::cout << name << " passed: 0x" << std::hex << std::setw(8) << std::setfill('0') << got << std::dec << std::endl;
            }
        };

        // RAM data: writes at 0x8000/0x8001/0x8002, read at same addresses
        if (tb.getCycleCount() == 1 && tb.ns_count == 10) {
            mem->write_en = 1;
            mem->addr = 0x8000;
            mem->data_in = 0xABCD;
            std::cout << "Setting RAM write 0x8000=0xABCD" << std::endl;
        }
        if (tb.getCycleCount() == 2 && tb.ns_count == 10) {
            mem->write_en = 1;
            mem->addr = 0x8001;
            mem->data_in = 0x5678;
            std::cout << "Setting RAM write 0x8001=0x5678" << std::endl;
        }
        if (tb.getCycleCount() == 3 && tb.ns_count == 10) {
            mem->write_en = 1;
            mem->addr = 0x8002;
            mem->data_in = 0x9ABC;
            std::cout << "Setting RAM write 0x8002=0x9ABC" << std::endl;
        }
        if (tb.getCycleCount() == 4 && tb.ns_count == 10) {
            mem->write_en = 0;
            mem->addr = 0x8000;
            mem->eval();
            check("RAM read 0x8000", mem->data_out, 0xFFFFABCD);
            mem->addr = 0x8001;
            mem->eval();
            check("RAM read 0x8001", mem->data_out, 0xFFFF5678);
            mem->addr = 0x8002;
            mem->eval();
            check("RAM read 0x8002", mem->data_out, 0xFFFF9ABC);
        }

        // RAM instruction read (instr_data_out) to verify interleaving across banks
        if (tb.getCycleCount() == 5 && tb.ns_count == 10) {
            mem->instr_addr = 0x8000;
            mem->eval();
            check("RAM instr read 0x8000", mem->instr_data_out, 0xABCD5678);
            mem->instr_addr = 0x8001;
            mem->eval();
            check("RAM instr read 0x8001", mem->instr_data_out, 0x56789ABC);
        }

        // VRAM test
        if (tb.getCycleCount() == 6 && tb.ns_count == 10) {
            mem->write_en = 1;
            mem->mem_vram_addr = 0x4000;
            mem->addr = 0x4000;
            mem->data_in = 0x1234;
            std::cout << "Setting VRAM write 0x4000=0x1234" << std::endl;
        }
        if (tb.getCycleCount() == 7 && tb.ns_count == 10) {
            mem->write_en = 0;
            mem->mem_vram_addr = 0x4000;
            mem->eval();
            check("VRAM read 0x4000", (uint32_t)mem->mem_vram_data, 0x1234);
        }

        // ROM programming writes: even->ROM1, odd->ROM2 (interleaved layout)
        if (tb.getCycleCount() == 8 && tb.ns_count == 10) {
            mem->mem_prog_write_en = 1;
            mem->instr_addr = 0x0000;
            mem->data_in = 0x5678;
            std::cout << "ROM write at 0x0000 => 0x5678" << std::endl;
        }
        if (tb.getCycleCount() == 9 && tb.ns_count == 10) {
            mem->mem_prog_write_en = 1;
            mem->instr_addr = 0x0001;
            mem->data_in = 0x9ABC;
            std::cout << "ROM write at 0x0001 => 0x9ABC" << std::endl;
        }
        if (tb.getCycleCount() == 10 && tb.ns_count == 10) {
            mem->mem_prog_write_en = 1;
            mem->instr_addr = 0x0002;
            mem->data_in = 0x5678;
            std::cout << "ROM write at 0x0002 => 0x5678" << std::endl;
        }
        if (tb.getCycleCount() == 11 && tb.ns_count == 10) {
            mem->mem_prog_write_en = 0;
            mem->addr = 0x0000;
            mem->instr_addr = 0x0000;
            mem->eval();
            check("ROM data 0x0000", mem->data_out, 0x56789ABC);
            check("ROM instr 0x0000", mem->instr_data_out, 0x56789ABC);
        }
        if (tb.getCycleCount() == 12 && tb.ns_count == 10) {
            mem->addr = 0x0001;
            mem->instr_addr = 0x0001;
            mem->eval();
            check("ROM data 0x0001", mem->data_out, 0x9ABC5678);
            check("ROM instr 0x0001", mem->instr_data_out, 0x9ABC5678);
        }
        if (tb.getCycleCount() == 13 && tb.ns_count == 10) {
            mem->addr = 0x0002;
            mem->instr_addr = 0x0002;
            mem->eval();
            check("ROM data 0x0002", mem->data_out, 0x56780000);
            check("ROM instr 0x0002", mem->instr_data_out, 0x56780000);
        }

        // Finish simulation loop actions
        mem->eval();
        if (tfp) tfp->dump(contextp->time());
        contextp->timeInc(1);


        if (tfp && tb.getCycleCount() % 20 == 0) tfp->flush();
    }

    // Simulation summary
    std::cout << "\n=== Simulation Complete ===" << std::endl;
    std::cout << "Total cycles: " << tb.getCycleCount() << std::endl;
    std::cout << "Simulation time: " << contextp->time() << " time units" << std::endl;

    if (tb.getCycleCount() >= max_cycles) {
        std::cout << "WARNING: Simulation stopped due to maximum cycle limit" << std::endl;
    }

    if (test_failed) {
        std::cout << "SOME TESTS FAILED" << std::endl;
    } else {
        std::cout << "ALL TESTS PASSED" << std::endl;
    }

    // Cleanup
    if (tfp) {
        tfp->close();
        delete tfp;
    }

    mem->final();
    delete mem;
    delete contextp;
    return 0;
}


