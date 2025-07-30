/* verilator lint_off MODDUP */
package Types;

    typedef struct packed {
        logic [3:0] opcode;
        logic imm_present;
        logic [2:0] funct;
    } instr_t;

    typedef struct packed {
        logic Sign;
        logic Overflow;
        logic Carry;
        logic Zero;
    } csr_t;

endpackage
