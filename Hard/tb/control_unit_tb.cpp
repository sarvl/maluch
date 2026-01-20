#include "Vcontrol_unit.h"
#include "verilated.h"
#include <cstdio>
#include <cstdlib>

// Global counter for failed tests
int failed_tests = 0;

// ------------------------------------------------------------
// Helper macros
// ------------------------------------------------------------
#define CHECK(cond, msg)                                   \
    do {                                                    \
        if (!(cond)) {                                     \
            printf("====[FAIL]==== %s <--------\n", msg);  \
            failed_tests++;                                \
        }                                                   \
    } while (0)

// ------------------------------------------------------------
// Instruction encoder
// OOOOJFFF'DDDDSSSS IIIIIIII'IIIIIIII
// ------------------------------------------------------------
uint32_t make_instr(uint8_t opcode, uint8_t funct,
                    uint8_t rd, uint8_t rs)
{
    return  (opcode << 28) |
            (0      << 27) |
            (funct  << 24) |
            (rd     << 20) |
            (rs     << 16);
}

// ------------------------------------------------------------
// Reset DUT state
// ------------------------------------------------------------
void reset(Vcontrol_unit* top)
{
    top->instruction = 0;
    top->reg_out1 = 0;
    top->reg_out2 = 0;
    top->io_data_r = 0;
    top->eval();
}

// ------------------------------------------------------------
// Execute instruction
// ------------------------------------------------------------
void exec_instr(Vcontrol_unit* top, uint32_t instr)
{
    top->instruction = instr;
    top->eval();
}

// ------------------------------------------------------------
// TEST: OUT instruction
// Opcode: 0111
// Behavior: IO[fff] <-- src
// ------------------------------------------------------------
void test_out(Vcontrol_unit* top)
{
    printf("Starting OUT instruction test\n");

    top->reg_out2 = 0x1234;
    exec_instr(top, make_instr(0b0111, 0b101, 0, 2));

    CHECK(top->io_w_en == 1, "OUT: io_w_en should be 1");
    CHECK(top->io_r_en == 0, "OUT: io_r_en should be 0");
    CHECK(top->io_addr == 0b101, "OUT: wrong io_addr");
    CHECK(top->io_data_w == 0x1234, "OUT: wrong io_data_w");

    printf("[FINISHED] OUT instruction\n\n");
}

// ------------------------------------------------------------
// TEST: IN instruction
// Opcode: 0110
// Behavior: Rd <-- IO[fff]
// ------------------------------------------------------------
void test_in(Vcontrol_unit* top)
{
    printf("Starting IN instruction test\n");

    exec_instr(top, make_instr(0b0110, 0b011, 4, 0));

    CHECK(top->io_r_en == 1, "IN: io_r_en should be 1");
    CHECK(top->io_w_en == 0, "IN: io_w_en should be 0");
    CHECK(top->io_addr == 0b011, "IN: wrong io_addr");

    // simulate IO device response
    if (top->io_r_en && top->io_addr == 0b011)
        top->io_data_r = 0xCAFE;

    top->eval();

    CHECK(top->reg_w_en == 1, "IN: reg_w_en should be 1");
    CHECK(top->reg_in == 0xCAFE, "IN: reg_in should equal io_data_r");

    printf("[FINISHED] IN instruction\n\n");
}

// ------------------------------------------------------------
// TEST: LDW instruction
// Opcode: 1000
// Behavior: Rd <-- MEM[src]
// ------------------------------------------------------------
void test_ldw(Vcontrol_unit* top)
{
    printf("Starting LDW instruction test\n");

    top->mem_ctrl_data_r = 0xDEAD;
    exec_instr(top, make_instr(0b1000, 0b000, 3, 2));

    CHECK(top->mem_ctrl_addres == top->reg_out2, "LDW: wrong memory address");
    CHECK(top->mem_ctrl_write_en == 0, "LDW: mem_ctrl_write_en should be 0");

    // Simulate memory controller response
    if (top->mem_ctrl_addres == top->reg_out2)
        top->mem_ctrl_data_r = 0xBEEF;

    top->eval();

    CHECK(top->reg_w_en == 1, "LDW: reg_w_en should be 1");
    CHECK(top->reg_in == 0xBEEF, "LDW: reg_in should equal memory data");

    printf("[FINISHED] LDW instruction\n\n");
}

// ------------------------------------------------------------
// TEST: STW instruction
// Opcode: 1001
// Behavior: MEM[src] <-- Rd
// ------------------------------------------------------------
void test_stw(Vcontrol_unit* top)
{
    printf("Starting STW instruction test\n");

    top->reg_out1 = 0xDEAD;
    exec_instr(top, make_instr(0b1001, 0b000, 3, 2));

    CHECK(top->mem_ctrl_addres == top->reg_out2, "STW: wrong memory address");
    CHECK(top->mem_ctrl_data_w == top->reg_out1, "STW: wrong memory data");
    CHECK(top->mem_ctrl_write_en == 1, "STW: mem_ctrl_write_en should be 1");

    printf("[FINISHED] STW instruction\n\n");
}

// ------------------------------------------------------------
// TEST: PUSH instruction
// Opcode: 1100
// Behavior: MEM[SP-1] <-- src, SP <-- SP-1
// ------------------------------------------------------------
void test_push(Vcontrol_unit* top) {
    printf("Starting PUSH instruction test\n");

    // Initialize inputs
    top->reg_out1 = 0x0010;
    top->reg_out2 = 0x00FF;
    exec_instr(top, make_instr(0b1100, 0b000, 0, 3));

    // Check ALU outputs for SP decrement
    CHECK(top->alu_ctrl == 0b001, "PUSH: ALU control mismatch");
    CHECK(top->src1 == 0x0010, "PUSH: ALU src1 mismatch");
    CHECK(top->src2 == 0x0001, "PUSH: ALU src2 mismatch");

     // Simulate ALU response
    if (top->alu_ctrl == 0b001) {
        top->alu_ret = top->src1 - top->src2;
        top->mem_ctrl_addres = top->alu_ret;
    }

    top->eval();

    // Check memory controller outputs for write
    CHECK(top->mem_ctrl_addres == 0x000F, "PUSH: Memory address mismatch");
    CHECK(top->mem_ctrl_data_w == 0x00FF, "PUSH: Memory data mismatch");
    CHECK(top->mem_ctrl_write_en == 1, "PUSH: Memory write enable mismatch");

    // Check register outputs for SP update
    CHECK(top->addr_in == 0b0010, "PUSH: Register address mismatch");
    CHECK(top->reg_in == 0x000F, "PUSH: Register data mismatch");
    CHECK(top->reg_w_en == 1, "PUSH: Register write enable mismatch");

    printf("[FINISHED] PUSH instruction\n\n");
}

// ------------------------------------------------------------
// TEST: PULL instruction
// Opcode: 1101
// Behavior: Rd <-- MEM[SP], SP <-- SP+1
// ------------------------------------------------------------
void test_pull(Vcontrol_unit* top) {
    printf("Starting PULL instruction test\n");

    // Initialize inputs
    top->reg_out1 = 0x00F0;
    top->mem_ctrl_data_r = 0xABBA;
    exec_instr(top, make_instr(0b1101, 0b000, 5, 0));

    // Check ALU settings (calculating SP + 1)
    CHECK(top->alu_ctrl == 0b000, "PULL: ALU control mismatch (should be ADD)");
    CHECK(top->src1 == 0x00F0, "PULL: ALU src1 mismatch (should be old SP)");
    CHECK(top->src2 == 0x0001, "PULL: ALU src2 mismatch");

    // Simulate ALU response
    if (top->alu_ctrl == 0b000) {
        top->alu_ret = top->src1 + top->src2; 
    }

    top->eval();

    // Check register file control signals
    CHECK(top->addr_out1 == 0b0010, "PULL: addr_out1 should be SP index (2)");
    CHECK(top->addr_in == 5, "PULL: addr_in mismatch");
    CHECK(top->reg_w_en == 1, "PULL: reg_w_en mismatch");
    CHECK(top->reg_in == 0xABBA, "PULL: reg_in mismatch (should be mem data)");
    
    // Check dedicated SP update signals
    CHECK(top->sp_w_en == 1, "PULL: sp_w_en mismatch");
    CHECK(top->sp_in == 0x00F1, "PULL: sp_in mismatch (should be SP+1)");

    // Check memory reading
    CHECK(top->mem_ctrl_addres == 0x00F0, "PULL: Memory address mismatch (old SP)");
    CHECK(top->mem_ctrl_write_en == 0, "PULL: Memory write enable mismatch");

    printf("[FINISHED] PULL instruction\n\n");
}

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
int main(int argc, char** argv)
{
    Verilated::commandArgs(argc, argv);
    Vcontrol_unit* top = new Vcontrol_unit;

    reset(top);

    test_out(top);
    test_in(top);
    test_ldw(top);
    test_stw(top);
    test_push(top);
    test_pull(top);

    printf("=====================================\n");
    printf("Simulation completed\n");
    printf("Failed tests: %d\n", failed_tests);
    printf("=====================================\n");

    delete top;
    return 0;
}