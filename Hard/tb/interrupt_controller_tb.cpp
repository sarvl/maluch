#include "Vinterrupt_controller.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

enum State {
    IDLE = 0,
    TRIGGER = 1
};

class Item {
    private:
        uint8_t masked_int_flags;
        uint8_t iret_f;
        uint8_t _reset;

        State current_state;

        uint8_t ref_irq_f;
        uint8_t ref_irq_id;

    public:
        Item() {
            current_state = IDLE;
        }

        uint8_t getFlags() { return masked_int_flags; }
        uint8_t getIret() { return iret_f; }
        uint8_t getReset() { return _reset; }
        uint8_t getRefIrqF() { return ref_irq_f; }
        uint8_t getRefIrqId() { return ref_irq_id; }
        State getState() { return current_state; }

        void init() {
            srand(time(0));
            current_state = IDLE;
        }

        void randomize() {
            masked_int_flags = rand() % 256;
            iret_f = (rand() % 100) < 30;
            _reset = (rand() % 100) < 1;
        }

        void step() {
            if      (masked_int_flags & 0x80) ref_irq_id = 0;
            else if (masked_int_flags & 0x40) ref_irq_id = 2;
            else if (masked_int_flags & 0x20) ref_irq_id = 4;
            else if (masked_int_flags & 0x10) ref_irq_id = 6;
            else if (masked_int_flags & 0x08) ref_irq_id = 8;
            else if (masked_int_flags & 0x04) ref_irq_id = 10;
            else if (masked_int_flags & 0x02) ref_irq_id = 12;
            else if (masked_int_flags & 0x01) ref_irq_id = 14;
            else ref_irq_id = 15;

            if (_reset) {
                current_state = IDLE;
            } else {
                
                if (current_state == TRIGGER) {
                    current_state = IDLE;
                } else if (masked_int_flags != 0) {
                    current_state = TRIGGER;
                } else {
                    current_state = IDLE;
                }
            }

            ref_irq_f = (current_state == TRIGGER);
        }

        bool verify(uint8_t dut_irq_f, uint8_t dut_irq_id) {
            return (dut_irq_f == ref_irq_f) && (dut_irq_id == ref_irq_id);
        }
};

int main(int argc, char* argv[]) {
    Verilated::commandArgs(argc, argv);
    VerilatedContext* contextp = new VerilatedContext;
    Vinterrupt_controller* top = new Vinterrupt_controller(contextp);

    int runs = 1000;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.find("n=") == 0) {
            runs = std::stoi(arg.substr(2));
        }
    }

    Item test_item;
    test_item.init();
    
    bool verbose = false;
    int successful_count = 0;
    int failed_count = 0;

    top->_reset = 1;
    top->clk = 0;
    top->eval();
    top->clk = 1;
    top->eval();
    top->_reset = 0;

    for (int i = 1; i <= runs; i++) {
        contextp->timeInc(1);
        
        test_item.randomize();
        
        top->clk = 0;
        top->masked_int_flags = test_item.getFlags();
        top->iret_f = test_item.getIret();
        top->_reset = test_item.getReset();
        top->eval();

        test_item.step();

        top->clk = 1;
        top->eval();

        bool successful = test_item.verify(top->irq_f, top->irq_relative_addr);
        
        if (successful) {
            successful_count++;
            if (verbose)
                printf("[PASS] %4d: State: %d | Flags: %02X | IRET: %d -> irq_f: %d id: %d\n",
                    i, test_item.getState(), test_item.getFlags(), test_item.getIret(), top->irq_f, top->irq_relative_addr);
        } else {
            failed_count++;
            printf("[FAIL] %4d: State: %d | Flags: %02X | IRET: %d | RST: %d\n", 
                i, test_item.getState(), test_item.getFlags(), test_item.getIret(), test_item.getReset());
            printf("       Expected: irq_f=%d, id=%d\n", test_item.getRefIrqF(), test_item.getRefIrqId());
            printf("       Got:      irq_f=%d, id=%d\n", top->irq_f, top->irq_relative_addr);
        }
    }

    printf("--------------------------------------\n");
    printf("Interrupt Controller Test\n");
    printf("Checks: %d \nSuccessful: %d (%3.2f %%)\nFailed: %d \n", 
        runs,
        successful_count, (float(successful_count)/runs)*100,
        failed_count
    );
    printf("--------------------------------------\n");

    top->final();
    delete top;
    delete contextp;
    return 0;
}
