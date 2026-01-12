#include "Vdecoder.h"
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
void reset(Vdecoder* top)
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
void exec_instr(Vdecoder* top, uint32_t instr)
{
    top->instruction = instr;
    top->eval();
}

// ------------------------------------------------------------
// TEST: OUT instruction
// Opcode: 0111
// Behavior: IO[fff] <-- src
// ------------------------------------------------------------
void test_out(Vdecoder* top)
{
    printf("Starting OUT instruction test\n");

    reset(top);

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
void test_in(Vdecoder* top)
{
    printf("Starting IN instruction test\n");

    reset(top);

    exec_instr(top, make_instr(0b0110, 0b011, 4, 0));

    CHECK(top->io_r_en == 1, "IN: io_r_en should be 1");
    CHECK(top->io_w_en == 0, "IN: io_w_en should be 0");
    CHECK(top->io_addr == 0b011, "IN: wrong io_addr");

    // simulate IO device response
    if (top->io_r_en && top->io_addr == 0b011)
        top->io_data_r = 0xCAFE;

    top->eval();

    CHECK(top->reg_w_en == 0, "IN: reg_w_en should be 1");
    CHECK(top->reg_in == 0xCAFE, "IN: reg_in should equal io_data_r");

    printf("[FINISHED] IN instruction\n\n");
}

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
int main(int argc, char** argv)
{
    Verilated::commandArgs(argc, argv);
    Vdecoder* top = new Vdecoder;

    test_out(top);
    test_in(top);

    printf("=====================================\n");
    printf("Simulation completed\n");
    printf("Failed tests: %d\n", failed_tests);
    printf("=====================================\n");

    delete top;
    return 0;
}