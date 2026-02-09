#include "Vprogram_counter.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include <iostream>
#include <vector>
#include <cstdlib>

// ------------------------------------------------------------
// Instruction encoder
// OOOOJFFF'DDDDSSSS IIIIIIII'IIIIIIII
// ------------------------------------------------------------
uint32_t make_instr(uint8_t opcode, uint8_t imm_valid, uint8_t funct,
                    uint8_t rd, uint8_t rs, uint16_t imm = 0)
{
    return  (opcode    << 28) |
            (imm_valid << 27) |
            (funct     << 24) |
            (rd        << 20) |
            (rs        << 16) |
            (imm       <<  0);
}

// Instruction definitions
struct TestDef {
    const char* name;
    uint8_t opcode;
    uint8_t funct;
};

TestDef test_defs[] = {
    {"JMP", 4, 0}, {"BEE", 4, 1}, {"BNE", 4, 2}, {"BGE", 4, 3},
    {"BLE", 4, 4}, {"BLL", 4, 5}, {"BGG", 4, 6}, {"BOO", 4, 7},
    {"BBS", 5, 0}, {"BSS", 5, 1}, {"BNS", 5, 2}, {"BAE", 5, 3},
    {"BBE", 5, 4}, {"BAA", 5, 5}, {"BBB", 5, 6}, {"BNO", 5, 7},
    {"CALL", 10, 0}, {"RET", 11, 0}
};

#define RUNS 20

bool check_condition(int code, uint8_t csr, uint8_t opcode) {
    bool Sign = (csr >> 3) & 1;
    bool Overflow = (csr >> 2) & 1;
    bool Carry = (csr >> 1) & 1;
    bool Zero = (csr >> 0) & 1;

    if (opcode == 10) { // CALL
        return true;
    } else if (opcode == 11) { // RET
        return true;
    }

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

    // Initialize inputs
    dut->instr_pointer = 0;
    dut->csr = 0;
    dut->instruction = 0;
    dut->instr_pointer_ctrl = 0;

    int all_succes = 0;
    int all_runs = 0;
    vluint64_t main_time = 0;

    dut->eval();
    tfp->dump(main_time);

    for (int i = 0; i < 18; i++) {
        int success_rate = 0;
        
        main_time += 10;
        tfp->dump(main_time);

        std::cout << "--- " << test_defs[i].name << " ---" << std::endl;

        for (int k = 0; k < RUNS; k++) {
            uint16_t val = rand() % 32000;
            uint16_t addr = rand() % 32000;
            uint8_t rand_csr = rand() & 0xF;
            
            // J determines if instruction is 32-bit (imm valid) or 16-bit
            uint8_t imm_valid = rand() & 1;

            int op_bit0 = test_defs[i].opcode & 1;
            int code = (op_bit0 << 3) | test_defs[i].funct;

            dut->instruction = make_instr(test_defs[i].opcode, imm_valid, test_defs[i].funct, 0, 0, val);
            dut->instr_pointer_ctrl = addr;
            dut->csr = rand_csr;

            dut->eval();
            
            main_time += 5;
            tfp->dump(main_time);

            bool taken = check_condition(code, rand_csr, test_defs[i].opcode);

            uint16_t expected_val;
            if (taken) {
                expected_val = imm_valid ? val : addr;
            } else {
                expected_val = dut->instr_pointer + (imm_valid ? 2 : 1);
            }

            if (dut->_nxt_instr_pointer == expected_val) {
                success_rate++;
            } else {
                 std::cout << "Failed: " << test_defs[i].name 
                           << " CSR=" << (int)rand_csr 
                           << " J=" << (int)imm_valid
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
