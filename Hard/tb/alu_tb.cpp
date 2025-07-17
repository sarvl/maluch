#include "Valu.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <filesystem>

enum aluoperand_t {
    ADD=0,
    SUB,
    AND,
    OR,
    XOR,
    NOT,
    LSR,
    LSL
};

class Item {
    private:
        int16_t src1, src2, ref_result = 0, mod_result;
        aluoperand_t aopcode = ADD;
    

    public:
        int16_t getSrc1() { return src1; }
        int16_t getSrc2() { return src2; }
        aluoperand_t getACode() { return aopcode; }
        int8_t getACodeValue() { return aopcode; }
        int16_t getReference() { return ref_result; }

        void setResult(int16_t val) {
            mod_result = val;
        }
        void init() {
            srand(time(0));
        }

        void randomize() {
            src1 = rand();
            src2 = rand();
            aopcode = aluoperand_t(rand() % 8);

            switch (aopcode) {
                case ADD:
                    ref_result = src1 + src2;
                    break;
                case SUB:
                    ref_result = src1 - src2;
                    break;
                case AND:
                    ref_result = src1 & src2;
                    break;
                case OR:
                    ref_result = src1 | src2;
                    break;
                case XOR:
                    ref_result = src1 ^ src2;
                    break;
                case NOT:
                    ref_result = ~src2;
                    break;
                case LSR:
                    ref_result = static_cast<uint16_t>(src2) >> 1;
                    break;
                case LSL:
                    ref_result = static_cast<uint16_t>(src2) << 1;
                    break;
            }
        }

        bool Verfiy() {
            return mod_result == ref_result;
        }
        bool Verfiy(int16_t result) {
            return result == ref_result;
        }
};


int main(int argc, char* argv[]) {
    Verilated::commandArgs(argc, argv);
    Valu* top = new Valu;

    int runs = 1000;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.find("n=") == 0) {
            runs = std::stoi(arg.substr(2));
        }
    }

    Item test_item;
    bool successful = false;
    int count[] = {0, 0};
    test_item.init();


    VerilatedVcdC* tfp = new VerilatedVcdC;
    Verilated::traceEverOn(true);
    top->trace(tfp, 2); 
    std::filesystem::create_directories("waveforms");
    tfp->open("waveforms/alu.vcd");
    vluint64_t main_time = 0;

    // Simulate for 20 cycles
    for (int i = 1; i <= runs; i++) {

        test_item.randomize();

        top->src1 = test_item.getSrc1();
        top->src2 = test_item.getSrc2();
        top->aopcode = test_item.getACodeValue();

        top->eval();  // evaluate model for current state

        successful = test_item.Verfiy(top->result);
        count[successful]++;

        // Optionally, print output signals at clock posedge
        if (successful) {
            printf("[PASS] %7d: opcode %1d; src1: %6d | src2: %6d | result: %6d \n",
                i, test_item.getACodeValue(),
                test_item.getSrc1(), 
                test_item.getSrc2(), 
                top->result);
        } else {
            printf("[FAIL] %7d: opcode %1d; src1: %6d | src2: %6d | result: %6d | expected: %6d \n",
                i, test_item.getACodeValue(),
                test_item.getSrc1(),
                test_item.getSrc2(),
                top->result, 
                test_item.getReference());
        }
        

        tfp->dump(main_time);  
        main_time++;
    }
    printf("--------------------------------------\n");
    printf("ALU test \n checks: %d \n succesful: %d (%3.2f %%)\n failed: %d \n", 
        runs,
        count[1], (float(count[1])/runs)*100,
        count[0]
    );
    printf("--------------------------------------\n");
    top->final();

    delete top;
    return 0;
}