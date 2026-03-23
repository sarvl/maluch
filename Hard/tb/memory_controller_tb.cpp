#include "Vmemory_controller.h"
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

class MemoryControllerTestBench {
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
    uint64_t max_cycles = 100;
    bool enable_trace = true;
    bool verbose = true;

    Verilated::commandArgs(argc, argv);
    VerilatedContext* contextp = new VerilatedContext;
    Vmemory_controller* mc = new Vmemory_controller(contextp);

    VerilatedFstC* tfp = nullptr;
    if (enable_trace) {
        Verilated::traceEverOn(true);
        tfp = new VerilatedFstC;
        mc->trace(tfp, 99);
        std::filesystem::create_directories("waveforms");
        tfp->open("waveforms/memory_controller.fst");
        std::cout << "Waveform tracing enabled: memory_controller.fst" << std::endl;
    }

    MemoryControllerTestBench tb;
    tb.reset();

    std::cout << "\n=== Starting Memory Controller Simulation ===" << std::endl;

    // Initial evaluation - INPUT signals only
    mc->clk = 0;
    mc->core2mem_instr_pointer = 0;
    mc->core2mem_addr = 0;
    mc->core2mem_data_w = 0;
    mc->core2mem_write_en = 0;
    mc->prog2mem_data = 0;
    mc->prog2mem_addr = 0;
    mc->prog2mem_w_en = 0;
    mc->prog2mem_stream_en = 0;
    mc->gpu2mem_addr = 0;
    // Simulate memory responses
    mc->mem_data_out = 0;
    mc->mem_instr_data_out = 0;
    mc->mem_vram_data = 0;
    mc->eval();
    if (tfp) tfp->dump(contextp->time());
    contextp->timeInc(1);

    // Test variables
    uint16_t core_addr = 0x8000;
    uint16_t core_data_w = 0x1234;
    uint16_t core_data_r = 0x1234;
    uint16_t prog_addr = 0x0000;
    uint16_t prog_data_1 = 0xABCD;
    uint16_t prog_data_2 = 0xEF01;
    uint16_t gpu_addr = 0x4000;
    uint16_t gpu_data = 0x2468;
    uint16_t instr_data = 0x1357;

    // Flags for tests
    bool test_failed = false;
    bool core_write_done = false;
    bool core_read_done = false;
    bool prog_write_1_done = false;
    bool prog_write_2_done = false;
    bool gpu_read_done = false;
    bool instr_fetch_done = false;

    auto check = [&](const char* name, uint32_t got, uint32_t exp) {
        if (got != exp) {
            std::cerr << name << " failed: got 0x" << std::hex << std::setw(8) << std::setfill('0') << got
                      << " exp 0x" << std::setw(8) << exp << std::dec << std::endl;
            test_failed = true;
        } else {
            std::cout << name << " passed: 0x" << std::hex << std::setw(8) << std::setfill('0') << got << std::dec << std::endl;
        }
    };

    // Main simulation loop
    while (tb.getCycleCount() < max_cycles) {
        mc->clk = tb.toggleClock();

        // Test 1: Core write operation
        if (tb.getCycleCount() == 1 && tb.ns_count == 10 && !core_write_done) {
            std::cout << "Test 1: Core write to 0x8000..." << std::endl;
            mc->core2mem_write_en = 1;
            mc->core2mem_addr = core_addr;
            mc->core2mem_data_w = core_data_w;
            mc->prog2mem_stream_en = 0;
            mc->eval();
            check("Core write addr", mc->mem_addr, core_addr);
            check("Core write data", mc->mem_data_in, core_data_w);
            check("Core write enable", mc->mem_write_en, 1);
            core_write_done = true;
        }
        
        // Test 2: Core read operation - setup read request
        if (tb.getCycleCount() == 2 && tb.ns_count == 10 && !core_read_done) {
            std::cout << "Test 2: Core read from 0x8000..." << std::endl;
            mc->core2mem_write_en = 0;
            mc->core2mem_addr = core_addr;
            mc->eval();
            core_read_done = true;
        }
        // Simulate memory delay: respond with data after read request
        // ram returns actual_data (16-bit)
        if (tb.getCycleCount() == 3 && tb.ns_count == 10 && core_read_done) {
            mc->mem_data_out = core_data_r;
            mc->eval();
            check("Core read data", mc->mem2core_data_r, core_data_r);
        }

        // Test 3: Programmer ROM write operation 1
        if (tb.getCycleCount() == 4 && tb.ns_count == 10 && !prog_write_1_done) {
            std::cout << "Test 3: Programmer ROM write 1 at 0x0000..." << std::endl;
            mc->prog2mem_stream_en = 1;
            mc->prog2mem_w_en = 1;
            mc->prog2mem_addr = prog_addr;
            mc->prog2mem_data = prog_data_1;
            mc->eval();
            check("Prog write addr 1", mc->mem_instr_addr, prog_addr);
            check("Prog write data 1", mc->mem_data_in, prog_data_1);
            check("Prog write_en 1", mc->mem_prog_write_en, 1);
            prog_write_1_done = true;
        }

        // Test 4: Programmer ROM write operation 2
        if (tb.getCycleCount() == 5 && tb.ns_count == 10 && !prog_write_2_done) {
            std::cout << "Test 4: Programmer ROM write 2 at 0x0001..." << std::endl;
            mc->prog2mem_w_en = 1;
            mc->prog2mem_addr = prog_addr + 1;
            mc->prog2mem_data = prog_data_2;
            mc->eval();
            check("Prog write addr 2", mc->mem_instr_addr, prog_addr + 1);
            check("Prog write data 2", mc->mem_data_in, prog_data_2);
            prog_write_2_done = true;
        }

        // Test 5: Instruction fetch
        if (tb.getCycleCount() == 6 && tb.ns_count == 10 && !instr_fetch_done) {
            std::cout << "Test 5: Instruction fetch from 0x0000..." << std::endl;
            mc->prog2mem_stream_en = 0;
            mc->prog2mem_w_en = 0;
            mc->core2mem_instr_pointer = prog_addr;
            mc->eval();
            instr_fetch_done = true;
        }
        if (tb.getCycleCount() == 7 && tb.ns_count == 10 && instr_fetch_done) {
            // Simulate memory returning the fetched instruction
            mc->mem_instr_data_out = ((uint32_t)prog_data_1 << 16) | prog_data_2;
            mc->eval();
            uint32_t expected_instr = ((uint32_t)prog_data_1 << 16) | prog_data_2;
            check("Instruction fetch result", mc->mem2core_instr, expected_instr);
        }

        // Test 6: GPU read operation
        if (tb.getCycleCount() == 8 && tb.ns_count == 10 && !gpu_read_done) {
            std::cout << "Test 6: GPU VRAM read from 0x4000..." << std::endl;
            mc->gpu2mem_addr = gpu_addr;
            mc->mem_vram_data = gpu_data;
            mc->eval();
            check("GPU VRAM address routing", mc->mem_vram_addr, gpu_addr);
            check("GPU data passthrough", mc->mem2gpu_data, gpu_data);
            gpu_read_done = true;
        }

        mc->eval();
        if (tfp) tfp->dump(contextp->time());
        contextp->timeInc(1);



        if (tb.getCycleCount() % 20 == 0 && tfp) tfp->flush();
    }

    // Simulation summary
    std::cout << "\n=== Simulation Complete ===" << std::endl;
    std::cout << "Total cycles: " << tb.getCycleCount() << std::endl;
    std::cout << "Simulation time: " << contextp->time() << " time units" << std::endl;

    if (tb.getCycleCount() >= max_cycles) {
        std::cout << "WARNING: Simulation stopped due to maximum cycle limit" << std::endl;
    }

    if (test_failed) {
        std::cout << "\nSOME TESTS FAILED" << std::endl;
    } else {
        std::cout << "\nALL TESTS PASSED" << std::endl;
    }

    // Cleanup
    if (tfp) {
        tfp->close();
        delete tfp;
    }

    mc->final();
    delete mc;
    delete contextp;
    return 0;
}
