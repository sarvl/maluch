// tb/keyboard_controller_tb.cpp
#include "Vps2_keyboard_controller.h"
#include "verilated.h"
#include "verilated_fst_c.h"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>

using namespace std;

VerilatedContext* contextp = nullptr;
Vps2_keyboard_controller* top = nullptr;
VerilatedFstC* tfp = nullptr;

vluint64_t main_time = 0;
double sc_time_stamp() { return main_time; }

// -----------------------
// Simulation utilities
// -----------------------
void fatal(const string &s) {
    cerr << "[FATAL] " << s << "\n";
    if (tfp) { tfp->close(); }
    if (top)  { top->final(); delete top; }
    if (contextp) delete contextp;
    exit(1);
}

void dump_and_advance() {
    if (tfp) tfp->dump(contextp->time());
    contextp->timeInc(1);
    main_time++;
}

// tick = one system clock period (low then high), dumping traces
void tick() {
    // clk low
    top->clk = 0;
    top->eval();
    dump_and_advance();

    // clk high
    top->clk = 1;
    top->eval();
    dump_and_advance();
}

// Poll until kb2io_int_f asserted or timeout tick cycles (returns true if asserted)
bool wait_for_int_or_timeout(int timeout_ticks) {
    while (top->kclk__en == 0 && top->kclk__out != 0) {
        tick();
    }
    for (int i = 0; i < timeout_ticks; ++i) {
        if (top->kb2io_int_f){
            return true;
        }

        tick();
    }
    return top->kb2io_int_f;
}

// -----------------------
// PS/2 driver (bit/byte)
// -----------------------
uint8_t ps2_parity(uint8_t b) {
    return (~(__builtin_parity(b))) & 1;
}

// Generate one PS/2 bit.
// The testbench actively drives kclk (inout) and kdata (input to module).
// Timing is coarse but stable: send some ticks with kclk low then high.
void ps2_bit(int b) {
    top->kdata = b ? 1 : 0;

    top->kclk = 1;
    for (int i = 0; i < 21; i++) {
        tick();
    }

    top->kclk = 0;
    for (int i = 0; i < 21; i++) {
        tick();
    }
}

void send_byte(uint8_t b) {
    uint8_t p = ps2_parity(b);

    ps2_bit(0); // start
    for (int i = 0; i < 8; ++i) ps2_bit( (b >> i) & 1 );
    ps2_bit(p); // parity
    ps2_bit(1); // stop (line released/high)
}

void send_make(uint8_t code) { send_byte(code); }

void send_break(uint8_t code) {
    send_byte(0xF0);
    send_byte(code);
}

// Read buffer directly from kb2io_data_r after the controller raises the interrupt flag.
// The controller only drives the bus while io2kb_r_en is asserted.
uint8_t read_data_register() {
    top->io2kb_r_en = 1;
    top->eval();
    tick();
    uint8_t val = (uint8_t)top->kb2io_data_r;
    top->io2kb_r_en = 0;
    tick();
    return val;
}

// -----------------------
// PASS/FAIL logger
// -----------------------
int tests = 0;
int passed = 0;

void log_result(bool ok, const string &name, int got, int expected) {
    ++tests;
    if (ok) ++passed;
    if (ok) {
        printf("[PASS] %-33s got=0x%02X (%02X)\n", name.c_str(), got, got);
    } else {
        printf("[FAIL] %-33s got=0x%02X expected=0x%02X\n", name.c_str(), got, expected);
    }
}

// -----------------------
// Tests
// -----------------------
void test_send_char(const string &name, uint8_t scancode, uint8_t expected_ascii) {
    // clear flags
    top->io2kb_r_en = 0;
    tick();

    send_make(scancode);
    for (int i = 0; i < 100; i++) {
        tick();
    }
    send_break(scancode);

    // wait for kb2io_int_f (controller interrupt) or timeout
    bool ok_int = wait_for_int_or_timeout(500);
    if (!ok_int) {
        log_result(false, name + " (no int)", 0xFF, expected_ascii);
        return;
    }

    // read data register
    uint8_t got = read_data_register();
    bool ok = (got == expected_ascii);
    log_result(ok, name, got, expected_ascii);
}

void test_shift_A() {
    // press shift (0x12), press A (0x1C) => expect 'A'
    top->io2kb_r_en = 0; tick();

    send_make(0x12); // shift
    for (int i = 0; i < 80; ++i) tick();

    send_make(0x1C); // 'a'
    send_break(0x1C);
    bool ok_int = wait_for_int_or_timeout(500);
    if (!ok_int) {
        log_result(false, "SHIFT + 'A' (no int)", 0xFF, 'A');
        return;
    }

    uint8_t got = read_data_register();
    log_result(got == 'A', "SHIFT + 'A'", got, 'A');

    // release shift (break)
    send_break(0x12);
    for (int i = 0; i < 50; ++i) tick();
}

void test_backspace() {
    top->io2kb_r_en = 0; tick();
    send_make(0x66); // backspace
    send_break(0x66);
    bool ok_int = wait_for_int_or_timeout(500);
    if (!ok_int) {
        log_result(false, "BACKSPACE (no int)", 0xFF, 0x08);
        return;
    }
    uint8_t got = read_data_register();
    log_result(got == 0x08, "BACKSPACE", got, 0x08);
}

void test_enter() {
    top->io2kb_r_en = 0; tick();
    send_make(0x5A); // Enter
    send_break(0x5A);
    bool ok_int = wait_for_int_or_timeout(500);
    if (!ok_int) {
        log_result(false, "ENTER (no int)", 0xFF, '\n');
        return;
    }
    uint8_t got = read_data_register();

    bool ok = (got == 0x0D) || (got == 0x0A);
    log_result(ok, "ENTER", got, 0x0D);
}

void test_space() {
    top->io2kb_r_en = 0; tick();
    send_make(0x29); // space
    send_break(0x29);
    bool ok_int = wait_for_int_or_timeout(500);
    if (!ok_int) {
        log_result(false, "SPACE (no int)", 0xFF, ' ');
        return;
    }
    uint8_t got = read_data_register();
    log_result(got == ' ', "SPACE", got, ' ');
}

void test_wrong_id_or_instruction() {
    top->io2kb_r_en = 0; tick();
    send_make(0x1C); // space
    send_break(0x1C);
    bool ok_int = wait_for_int_or_timeout(500);
    if (!ok_int) {
        log_result(false, "a (no int)", 0xFF, 'a');
        return;
    }
    uint8_t got = read_data_register();
    log_result(got == 'a', "test id and instruction on 'a'", got, 'a');
}

// fuzz: send random make codes from given vector and verify that controller returns something (int + data).
void fuzz_test(int runs) {
    vector<uint8_t> pool = {
        0x1C, // a
        0x32, // b
        0x21, // c
        0x23, // d
        0x24, // e
        0x2B, // f
        0x34, // g
        0x33, // h
        0x43, // i
        0x3B, // j
        0x42, // k
        0x4B, // l
        0x3A, // m
        0x31, // n
        0x44, // o
        0x4D, // p
        0x15, // q
        0x2D, // r
        0x1B, // s
        0x2C, // t
        0x3C, // u
        0x2A, // v
        0x1D, // w
        0x22, // x
        0x35, // y
        0x1A, // z
        //0x12, // left shift
        0x59, // right shift
        0x66, // backspace
        0x29, // space
        0x5A  // enter
    };

    for (int i = 0; i < runs; ++i) {
        uint8_t sc = pool[rand() % pool.size()];

        send_make(sc);
        send_break(sc);

        bool ok_int = wait_for_int_or_timeout(300);
        if (!ok_int) {
            log_result(false, "FUZZ no int", 0xFF, 0x00);
            continue;
        }

        uint8_t got = read_data_register();
        log_result(true, "FUZZ", got, got);
    }
}

// -----------------------
// Main
// -----------------------
int main(int argc, char** argv) {

    int fuzz_runs = 100;

    srand((unsigned)time(nullptr));

    contextp = new VerilatedContext;
    contextp->debug(0);
    contextp->randSeed(0);

    top = new Vps2_keyboard_controller(contextp);

    // Enable tracing
    Verilated::traceEverOn(true);
    tfp = new VerilatedFstC;
    top->trace(tfp, 99);
    // create directory if needed and open file
    system("mkdir -p waveforms >/dev/null 2>&1");
    tfp->open("waveforms/keyboard.fst");

    // --- Initial conditions ---
    top->kclk = 1;
    top->kdata = 1;
    top->io2kb_r_en = 0;
    top->clk = 0;
    top->rstn = 1;   // start released

    // --- Apply asynchronous reset ---
    top->rstn = 0;   // assert reset
    tick();          // reset is async, but one cycle helps dumping
    tick();
    tick();
    top->kclk = 1;

    top->rstn = 1;   // deassert reset
    tick();
    tick();
    tick();

    // after reset, wait a little
    for (int i = 0; i < 30; ++i) tick();

    printf("Starting keyboard_controller tests\n");

    // Test 1: 'a' (0x1C) => expected ASCII 'a' (0x61)
    test_send_char("SEND 'a'", 0x1C, 'a');

    // Test 2: SHIFT + 'A'
    test_shift_A();

    // Test 3: BACKSPACE
    test_backspace();

    // Test 4: ENTER
    test_enter();

    // Test 5: SPACE
    test_space();

    // Test 6: ID or instruction
    test_wrong_id_or_instruction();

    // Optional fuzz tests
    if (fuzz_runs > 0) {
        printf("\nStarting fuzz test runs\n");
        fuzz_test(fuzz_runs);
        printf("\nEnded fuzz test runs: %d\n", fuzz_runs);
    }

    // Summary
    printf("\n-----------------------------\n");
    printf("Keyboard test results:\n");
    printf("  Total tests: %d\n", tests);
    printf("  Passed:      %d\n", passed);
    printf("  Failed:      %d\n", tests - passed);
    printf("-----------------------------\n");

    // finish
    tfp->close();
    top->final();

    delete tfp;
    delete top;
    delete contextp;

    return 0;
}