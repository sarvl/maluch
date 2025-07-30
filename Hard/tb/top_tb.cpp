#include "Vtop.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>


class Item {
private:
    std::vector<uint32_t> instructions;
    size_t pc = 0;
    bool clk = false;
    uint64_t cycle_count = 0;
    
public:
    bool loadInstructions(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return false;
        }
        
        std::string line;
        instructions.clear();
        
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#' || line[0] == '/') {
                continue;
            }
            
            // Remove whitespace
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
            
            try {
                // Convert binary string to integer
                uint32_t instr = std::stoul(line, nullptr, 2);
                instructions.push_back(instr);
                std::cout << "Loaded instruction " << instructions.size() 
                          << ": 0x" << std::hex << instr << " (binary: " << line << ")" << std::dec << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error parsing instruction: " << line << " - " << e.what() << std::endl;
                return false;
            }
        }
        
        std::cout << "Loaded " << instructions.size() << " instructions" << std::endl;
        return !instructions.empty();
    }
    
    uint32_t getCurrentInstruction() {
        if (pc >= instructions.size()) {
            return 0; // End of program
        }
        return instructions[pc];
    }
    
    void nextInstruction() {
        pc++;
    }
    
    bool hasMoreInstructions() {
        return pc < instructions.size();
    }
    
    bool toggleClock() {
        clk = !clk;
        if (clk) { // Rising edge
            cycle_count++;
        }
        return clk;
    }
    
    uint64_t getCycleCount() const {
        return cycle_count;
    }
    
    void reset() {
        pc = 0;
        clk = false;
        cycle_count = 0;
    }
    
    void printStatus() {
        std::cout << "Cycle: " << cycle_count << ", PC: " << pc 
                  << ", Current Instr: 0x" << std::hex << getCurrentInstruction() << std::dec << std::endl;
    }
};


int main(int argc, char* argv[]) {

    std::string code_file = "tb/code.txt";
    uint64_t max_cycles = 10000;
    bool enable_trace = true;
    bool verbose = true;

    Verilated::commandArgs(argc, argv);
    VerilatedContext* contextp = new VerilatedContext;
    Vtop* cpu = new Vtop(contextp);


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
    cpu->instr_in = 0;
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
        
        // On rising edge, feed new instruction
        if (cpu->clk) {
            if (tb.hasMoreInstructions()) {
                cpu->instr_in = tb.getCurrentInstruction();
                if (verbose) {
                    std::cout << "Cycle " << tb.getCycleCount() 
                              << ": Feeding instruction 0x" << std::hex << cpu->instr_in << std::dec << std::endl;
                }
                tb.nextInstruction();
                instructions_executed++;
            } else {
                cpu->instr_in = 0; // NOP or halt
                if (verbose) {
                    std::cout << "Cycle " << tb.getCycleCount() << ": No more instructions (NOP)" << std::endl;
                }
            }
        }
        
        // Evaluate the CPU
        cpu->eval();
        
        // Dump waveform if tracing
        if (tfp) {
            tfp->dump(contextp->time());
        }
        
        // Check for simulation end conditions
        if (!tb.hasMoreInstructions()) {
            // Run a few more cycles after last instruction to let CPU finish
            static uint32_t post_instruction_cycles = 0;
            post_instruction_cycles++;
            if (post_instruction_cycles > 10) {
                simulation_complete = true;
                std::cout << "All instructions processed, ending simulation." << std::endl;
            }
        }
        
        // Print periodic status
        if (verbose && tb.getCycleCount() % 100 == 0) {
            tb.printStatus();
        }
        
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