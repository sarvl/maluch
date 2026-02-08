#include "Vprogram_counter.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include <iostream>
#include <vector>
#include <cstdlib>

// Instruction definitions
#define JMP 0b0100000000000000
#define BEE 0b0100000100000000
#define BNE 0b0100001000000000
#define BGE 0b0100001100000000
#define BLE 0b0100010000000000
#define BLL 0b0100010100000000
#define BGG 0b0100011000000000
#define BOO 0b0100011100000000
#define BBS 0b0101000000000000
#define BSS 0b0101000100000000
#define BNS 0b0101001000000000
#define BAE 0b0101001100000000
#define BBE 0b0101010000000000
#define BAA 0b0101010100000000
#define BBB 0b0101011000000000
#define BNO 0b0101011100000000

#define RUNS 20

bool check_condition(int code, uint8_t csr) {
    bool Sign = (csr >> 3) & 1;
    bool Overflow = (csr >> 2) & 1;
    bool Carry = (csr >> 1) & 1;
    bool Zero = (csr >> 0) & 1;

    switch (code) {
        case 0b0000: return true;
        case 0b0001: return Zero;
        case 0b0010: return !Zero;
        case 0b0011: return !(Sign ^ Overflow);
        case 0b0100: return Zero & (Sign ^ Overflow);
        case 0b0101: return !Zero & !(Sign ^ Overflow);
        case 0b0110: return (Sign ^ Overflow);
        case 0b0111: return Overflow;
        case 0b1000: return false; // Busy
        case 0b1001: return Sign;
        case 0b1010: return !Sign;
        case 0b1011: return !Carry;
        case 0b1100: return Zero | Carry;
        case 0b1101: return !(Zero | Carry);
        case 0b1110: return Carry;
        case 0b1111: return !Overflow;
        default: return false;
    }
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true);

    Vprogram_counter* dut = new Vprogram_counter;
    VerilatedFstC* tfp = new VerilatedFstC;
    dut->trace(tfp, 99);
    tfp->open("waveforms/program_counter.fst");

    uint16_t test_code[] = {
        JMP, BEE, BNE, BGE, BLE, BLL, BGG, BOO,
        BBS, BSS, BNS, BAE, BBE, BAA, BBB, BNO
    };

    const char* test_name[] = {
        "JMP", "BEE", "BNE", "BGE", "BLE", "BLL", "BGG", "BOO",
        "BBS", "BSS", "BNS", "BAE", "BBE", "BAA", "BBB", "BNO"
    };

    // Initialize inputs
    dut->instr_pointer = 1;
    dut->csr = 0;
    dut->instruction = 0;
    dut->instr_pointer_ctrl = 0;

    int all_succes = 0;
    int all_runs = 0;
    vluint64_t main_time = 0;

    dut->eval();
    tfp->dump(main_time);

    for (int i = 0; i < 16; i++) {
        int success_rate = 0;
        
        main_time += 10;
        tfp->dump(main_time);

        std::cout << "--- " << test_name[i] << " ---" << std::endl;

        for (int k = 0; k < RUNS; k++) {
            uint16_t addr = rand() % 32000;

            uint8_t rand_csr = rand() & 0xF; 

            dut->instruction = (test_code[i] << 16) | addr;
            dut->instr_pointer_ctrl = addr;
            dut->csr = rand_csr;

            dut->eval();
            
            main_time += 5;
            tfp->dump(main_time);

            int op_bit0 = (test_code[i] >> 12) & 1;
            int funct3 = (test_code[i] >> 8) & 0x7;
            int code = (op_bit0 << 3) | funct3;

            bool taken = check_condition(code, rand_csr);
            uint16_t expected_val = taken ? addr : (dut->instr_pointer + 2);

            if (dut->_nxt_instr_pointer == expected_val) {
                success_rate++;
            } else {
                 std::cout << "Failed: " << test_name[i] 
                           << " CSR=" << (int)rand_csr 
                           << " Expected=" << expected_val 
                           << " Got=" << dut->_nxt_instr_pointer << std::endl;
            }
        }

        std::cout << "  Sucess rate: " << ((float)success_rate/RUNS * 100) << " %" << std::endl;
        std::cout << "  Failed: " << (RUNS-success_rate) << std::endl;
        all_succes += success_rate;
        all_runs += RUNS;
    }

    main_time += 20;
    tfp->dump(main_time);

    std::cout << "-----  SUMMARY  -----" << std::endl;
    std::cout << "   Runs: " << all_runs << std::endl;
    std::cout << "   Commands: " << (all_runs/RUNS) << std::endl;
    std::cout << "   Overal succes: " << ((float)all_succes/all_runs * 100) << " %" << std::endl;
    std::cout << "   Overal fails: " << (all_runs-all_succes) << std::endl;
    std::cout << "   Command fails: " << ((all_runs-all_succes)/RUNS) << std::endl;

    dut->final();
    tfp->close();
    delete dut;
    delete tfp;
    return 0;
}
