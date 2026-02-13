#include "Vcore.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "src/progmem.h"


class Item {
private:
    FlashMemmory16* Memmory = new FlashMemmory16;
    bool clk = false;
    uint64_t cycle_count = 0;
    
public:
    uint64_t ns_count = 0;

    bool loadInstructions(const std::string& filename) {
        return Memmory->program(filename);
    }
    
    uint32_t getInstruction(uint16_t pc) {
        return Memmory->read32(pc);
    }

    uint16_t getData(uint16_t address) {
        return Memmory->read(address);
    }

    void writeData(uint16_t address, uint16_t data) {
        Memmory->write(address, data);
    }
    
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
    }
    
    void printStatus() {
        std::cout << "Cycle: " << cycle_count << std::dec << std::endl;
    }

    ~Item() {
        delete Memmory;
    }
};


int main(int argc, char* argv[]) {

    std::string code_file = "tb/code.txt";
    uint64_t max_cycles = 100;
    bool enable_trace = true;
    bool verbose = true;

    Verilated::commandArgs(argc, argv);
    VerilatedContext* contextp = new VerilatedContext;
    Vcore* cpu = new Vcore(contextp);


    VerilatedFstC* tfp = nullptr;
    if (enable_trace) {
        Verilated::traceEverOn(true);
        tfp = new VerilatedFstC;
        cpu->trace(tfp, 99);
        std::filesystem::create_directories("waveforms");
        tfp->open("waveforms/core.fst");
        std::cout << "Waveform tracing enabled: core.fst" << std::endl;
    }


    Item tb;
    if (!tb.loadInstructions(code_file)) {
        std::cerr << "Failed to load instructions from " << code_file << std::endl;
        return 1;
    }

    std::cout << "\n=== Starting CPU Simulation ===" << std::endl;
    
    // Reset sequence
    std::cout << "Applying reset..." << std::endl;
    cpu->_reset = 1;
    cpu->clk = 0;
    cpu->mem2core_instr = 0;
    cpu->eval();
    if (tfp) tfp->dump(contextp->time());
    contextp->timeInc(1);
    
    // Hold reset for a few cycles
    for (int i = 0; i < 5; i++) {
        cpu->clk = tb.toggleClock();
        cpu->eval();
        if (tfp) tfp->dump(contextp->time());
        contextp->timeInc(1);
    }
    
    // Release reset
    cpu->_reset = 0;
    std::cout << "Reset released, starting execution..." << std::endl;
    
    // Main simulation loop
    bool simulation_complete = false;
    uint32_t instructions_executed = 0;
    
    while (!simulation_complete && tb.getCycleCount() < max_cycles) {
        // Toggle clock
        cpu->clk = tb.toggleClock();
        
        // Feed new instruction slightly after rising edge
        uint32_t instr = tb.getInstruction(static_cast<uint16_t>(cpu->core2mem_instr_pointer));
        
        if (tb.ns_count == 3) { // Simulating instruction fetch delay
            cpu->mem2core_instr = static_cast<uint32_t>(instr);
            if (verbose) {
                std::cout << "Cycle " << std::dec << (tb.getCycleCount() + 1) 
                            << ": Pointer 0x" << std::hex << std::setw(8) << std::setfill('0') << cpu->core2mem_instr_pointer
                            << " instruction to execute: 0x" << cpu->mem2core_instr << std::endl;
            }
            instructions_executed++;
        }

        // Evaluate the CPU
        cpu->eval();

        if (!cpu->clk && cpu->core2mem_w_en && tb.ns_count == 13) { // Simulating memory write delay
            std::cout << "Memory write: Address 0x" << std::hex << std::setw(4) << std::setfill('0') << cpu->core2mem_addr
                      << " Data 0x" << std::hex << std::setw(4) << std::setfill('0') << cpu->core2mem_data_w << std::dec << std::endl;
            tb.writeData(static_cast<uint16_t>(cpu->core2mem_addr), static_cast<uint16_t>(cpu->core2mem_data_w));
        }

        cpu->mem2core_data_r = tb.getData(static_cast<uint16_t>(cpu->core2mem_addr));

        // Evaluate the CPU
        cpu->eval();
        
        // Dump waveform if tracing
        if (tfp) {
            tfp->dump(contextp->time());
        }
        
        // Print periodic status
        if (verbose && tb.getCycleCount() % 100 == 0) {
            tb.printStatus();
        }

        if (tb.getCycleCount() %20 == 0 ) tfp->flush();
        
        contextp->timeInc(1);
    }
    
    // Simulation summary
    std::cout << "\n=== Simulation Complete ===" << std::endl;
    std::cout << "Total cycles: " << tb.getCycleCount() << std::endl;
    std::cout << "Instructions executed: " << instructions_executed << std::endl;
    std::cout << "Simulation time: " << contextp->time() << " time units" << std::endl;
    
    if (tb.getCycleCount() >= max_cycles) {
        std::cout << "WARNING: Simulation stopped due to maximum cycle limit" << std::endl;
    }
    
    // Cleanup
    if (tfp) {
        tfp->close();
        delete tfp;
    }
    
    cpu->final();
    delete cpu;
    delete contextp;
    return 0;
}