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
    exec_instr(top, make_instr(0b0111, 0, 0b101, 0, 2));

    CHECK(top->io_w_en == 1, "OUT: io_w_en should be 1");
    CHECK(top->io_r_en == 0, "OUT: io_r_en should be 0");
    CHECK(top->io_addr == 0b101, "OUT: wrong io_addr");
    CHECK(top->io_data_w == 0x1234, "OUT: wrong io_data_w");

    // Test Immediate version
    printf("Starting OUT immediate instruction test\n");
    exec_instr(top, make_instr(0b0111, 1, 0b101, 0, 0, 0x5678));
    CHECK(top->io_w_en == 1, "OUT(I): io_w_en should be 1");
    CHECK(top->io_addr == 0b101, "OUT(I): wrong io_addr");
    CHECK(top->io_data_w == 0x5678, "OUT(I): wrong io_data_w (should be imm)");

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

    exec_instr(top, make_instr(0b0110, 0, 0b011, 4, 0));

    CHECK(top->io_r_en == 1, "IN: io_r_en should be 1");
    CHECK(top->io_w_en == 0, "IN: io_w_en should be 0");
    CHECK(top->io_addr == 0b011, "IN: wrong io_addr");

    // simulate IO device response
    if (top->io_r_en && top->io_addr == 0b011)
        top->io_data_r = 0xCAFE;

    top->eval();

    CHECK(top->reg_w_en == 1, "IN: reg_w_en should be 1");
    CHECK(top->reg_in == 0xCAFE, "IN: reg_in should equal io_data_r");

    // Test Immediate version
    printf("IN instruction doesn't use immediate\n");

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
    exec_instr(top, make_instr(0b1000, 0, 0b000, 3, 2));

    CHECK(top->mem_ctrl_addres == top->reg_out2, "LDW: wrong memory address");
    CHECK(top->mem_ctrl_write_en == 0, "LDW: mem_ctrl_write_en should be 0");

    // Simulate memory controller response
    if (top->mem_ctrl_addres == top->reg_out2)
        top->mem_ctrl_data_r = 0xBEEF;

    top->eval();

    CHECK(top->reg_w_en == 1, "LDW: reg_w_en should be 1");
    CHECK(top->reg_in == 0xBEEF, "LDW: reg_in should equal memory data");

    // Test Immediate version
    printf("Starting LDW immediate instruction test\n");
    exec_instr(top, make_instr(0b1000, 1, 0b000, 3, 0, 0x1234));
    
    CHECK(top->mem_ctrl_addres == 0x1234, "LDW(I): address should be imm");
    
    if (top->mem_ctrl_addres == 0x1234)
        top->mem_ctrl_data_r = 0xAA55;
    top->eval();

    CHECK(top->reg_w_en == 1, "LDW(I): reg_w_en should be 1");
    CHECK(top->reg_in == 0xAA55, "LDW(I): reg_in mismatch");

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
    exec_instr(top, make_instr(0b1001, 0, 0b000, 3, 2));

    CHECK(top->mem_ctrl_addres == top->reg_out2, "STW: wrong memory address");
    CHECK(top->mem_ctrl_data_w == top->reg_out1, "STW: wrong memory data");
    CHECK(top->mem_ctrl_write_en == 1, "STW: mem_ctrl_write_en should be 1");

    // Test Immediate version
    printf("Starting STW immediate instruction test\n");
    top->reg_out1 = 0xD1C3;
    exec_instr(top, make_instr(0b1001, 1, 0b000, 3, 0, 0x4321));

    CHECK(top->mem_ctrl_addres == 0x4321, "STW(I): address should be imm");
    CHECK(top->mem_ctrl_data_w == 0xD1C3, "STW(I): wrong memory data");
    CHECK(top->mem_ctrl_write_en == 1, "STW(I): mem_ctrl_write_en should be 1");

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
    exec_instr(top, make_instr(0b1100, 0, 0b000, 0, 3));

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
    CHECK(top->sp_w_en == 1, "PUSH: sp_w_en mismatch");
    CHECK(top->sp_in == top->alu_ret, "PUSH: sp_in mismatch (should be SP-1)");

    // Test Immediate version (Push Constant)
    printf("Starting PUSH immediate instruction test\n");
    exec_instr(top, make_instr(0b1100, 1, 0b000, 0, 0, 0x9999));
    
    // Check ALU usage for SP decrement (SP-1)
    CHECK(top->alu_ctrl == 0b001, "PUSH(I): ALU control mismatch (SUB)");
    CHECK(top->src2 == 1, "PUSH(I): ALU src2 should be 1 for SP decrement");
    
    // Memory data should be the immediate
    CHECK(top->mem_ctrl_data_w == 0x9999, "PUSH(I): memory data should be imm");
    CHECK(top->mem_ctrl_write_en == 1, "PUSH(I): Memory write enable mismatch");

    if (top->alu_ctrl == 0b001) {
        top->alu_ret = top->src1 - top->src2;
        top->mem_ctrl_addres = top->alu_ret;
    }
    top->eval();
    
    // SP update check
    CHECK(top->sp_w_en == 1, "PUSH(I): SP write enable");
    CHECK(top->sp_in == top->alu_ret, "PUSH(I): SP update value");

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
    exec_instr(top, make_instr(0b1101, 0, 0b000, 5, 0));

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
    CHECK(top->sp_in == top->alu_ret, "PULL: sp_in mismatch (should be SP+1)");

    // Check memory reading
    CHECK(top->mem_ctrl_addres == 0x00F0, "PULL: Memory address mismatch (old SP)");
    CHECK(top->mem_ctrl_write_en == 0, "PULL: Memory write enable mismatch");

    // Test Immediate version
    printf("PULL instruction doesn't use immediate\n");

    printf("[FINISHED] PULL instruction\n\n");
}

// ------------------------------------------------------------
// TEST: BRANCH instruction
// Opcode: 0100
// Behavior: PC <-- Rs
// ------------------------------------------------------------
void test_branch(Vcontrol_unit* top) {
    printf("Starting BRANCH instructions test\n");

    top->reg_out2 = 0xB00B;
    exec_instr(top, make_instr(0b0100, 0, 0b000, 0, 5));

    CHECK(top->addr_out2 == 5, "BRANCH: addr_out2 mismatch");
    CHECK(top->instr_pointer_ctrl == 0xB00B, "BRANCH: instr_pointer_ctrl mismatch");

    top->reg_out2 = 0xB00F;
    exec_instr(top, make_instr(0b0101, 0, 0b000, 0, 7));

    CHECK(top->addr_out2 == 7, "BRANCH: addr_out2 mismatch");
    CHECK(top->instr_pointer_ctrl == 0xB00F, "BRANCH: instr_pointer_ctrl mismatch");

    // Test Immediate version
    printf("Starting BRANCH immediate instruction test\n");
    exec_instr(top, make_instr(0b0100, 1, 0b000, 0, 0, 0x8888));
    CHECK(top->instr_pointer_ctrl == 0x8888, "BRANCH(I): PC should be imm");

    printf("[FINISHED] BRANCH instructions\n\n");
}

// ------------------------------------------------------------
// TEST: CALL instruction
// Opcode: 1010
// Behavior: MEM[SP-1] <-- PC_seq, SP <-- SP-1, PC <-- Rs
// ------------------------------------------------------------
void test_call(Vcontrol_unit* top) {
    printf("Starting CALL instruction test\n");

    // Initialize inputs
    top->reg_out1 = 0x0200;
    top->reg_out2 = 0xF00D;
    top->instr_pointer_seq = 0x0084;
    exec_instr(top, make_instr(0b1010, 0, 0b000, 0, 6));

    // Check PC update (Jump to target)
    CHECK(top->instr_pointer_ctrl == 0xF00D, "CALL: instr_pointer_ctrl mismatch");

    // Check ALU settings (Calculate new SP: SP - 1)
    CHECK(top->alu_ctrl == 0b001, "CALL: ALU control mismatch (should be SUB)");
    CHECK(top->src1 == 0x0200, "CALL: ALU src1 mismatch (SP)");
    CHECK(top->src2 == 0x0001, "CALL: ALU src2 mismatch");

    // Simulate ALU calculation
    if (top->alu_ctrl == 0b001) {
        top->alu_ret = top->src1 - top->src2; // 0x01FF
    }

    top->eval();

    // Check Memory write (Push Return Address)
    CHECK(top->mem_ctrl_addres == 0x01FF, "CALL: Memory address mismatch (New SP)");
    CHECK(top->mem_ctrl_data_w == 0x0084, "CALL: Memory write data mismatch (Return Address)");
    CHECK(top->mem_ctrl_write_en == 1, "CALL: Memory write enable mismatch");

    // Check Registers/SP update
    CHECK(top->addr_out1 == 0b0010, "CALL: addr_out1 should be SP index (2)");
    CHECK(top->addr_out2 == 6, "CALL: addr_out2 should be src_reg (6)");
    CHECK(top->sp_w_en == 1, "CALL: sp_w_en mismatch");
    CHECK(top->sp_in == top->alu_ret, "CALL: sp_in mismatch (New SP)");

    // Test Immediate version
    printf("Starting CALL immediate instruction test\n");
    top->reg_out1 = 0x01FF; // Update SP for next test
    top->instr_pointer_seq = 0x0090;
    exec_instr(top, make_instr(0b1010, 1, 0b000, 0, 0, 0x4000));

    CHECK(top->instr_pointer_ctrl == 0x4000, "CALL(I): PC should be imm");
    
    // Check SP decrement logic remains correct
    CHECK(top->alu_ctrl == 0b001, "CALL(I): ALU control SUB");
    CHECK(top->src2 == 1, "CALL(I): src2 should be 1");
    // Check Mem Write
    CHECK(top->mem_ctrl_write_en == 1, "CALL(I): Mem write enable");
    
    if (top->alu_ctrl == 0b001) {
        top->alu_ret = top->src1 - top->src2;
    }
    top->eval();
    
    CHECK(top->sp_in == top->alu_ret, "CALL(I): sp_in mismatch");

    printf("[FINISHED] CALL instruction\n\n");
}

// ------------------------------------------------------------
// TEST: RET instruction
// Opcode: 1011
// Behavior: PC <-- MEM[SP], SP <-- SP+1
// ------------------------------------------------------------
void test_ret(Vcontrol_unit* top) {
    printf("Starting RET instruction test\n");

    // Initialize inputs
    top->reg_out1 = 0x01FE; // Current SP
    top->mem_ctrl_data_r = 0xBAAD; // Return Address located on stack
    
    exec_instr(top, make_instr(0b1011, 0, 0b000, 0, 0));

    // Check PC update (Jump to Return Address read from memory)
    CHECK(top->instr_pointer_ctrl == 0xBAAD, "RET: instr_pointer_ctrl mismatch (should be mem data)");

    // Check Memory Read Address (Read from current SP)
    CHECK(top->mem_ctrl_addres == 0x01FE, "RET: Memory address mismatch (must match current SP)");
    CHECK(top->mem_ctrl_write_en == 0, "RET: Memory write enable mismatch (should be read)");

    // Check ALU settings (Calculate SP+1)
    CHECK(top->alu_ctrl == 0b000, "RET: ALU control mismatch (should be ADD)");
    CHECK(top->src1 == 0x01FE, "RET: ALU src1 mismatch (SP)");
    CHECK(top->src2 == 0x0001, "RET: ALU src2 mismatch");

    // Simulate ALU calculation
    if (top->alu_ctrl == 0b000) {
        top->alu_ret = top->src1 + top->src2; // 0x01FF
    }
    top->eval();

    // Check SP Update
    CHECK(top->addr_out1 == 0b0010, "RET: addr_out1 should be SP index (2)");
    CHECK(top->sp_w_en == 1, "RET: sp_w_en mismatch");
    CHECK(top->sp_in == top->alu_ret, "RET: sp_in mismatch (SP+1)");

    // Test Immediate version
    printf("RET instruction doesn't use immediate\n");

    printf("[FINISHED] RET instruction\n\n");
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
    test_branch(top);
    test_call(top);
    test_ret(top);

    printf("=====================================\n");
    printf("Simulation completed\n");
    printf("Failed tests: %d\n", failed_tests);
    printf("=====================================\n");

    delete top;
    return 0;
}