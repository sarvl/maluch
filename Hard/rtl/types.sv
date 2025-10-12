/* verilator lint_off MODDUP */
package types;

    typedef struct packed {
        logic Sign;
        logic Overflow;
        logic Carry;
        logic Zero;
        logic Busy;
    } csr_t;

    typedef struct packed {
        logic [3:0]     opcode;
        logic           imm_valid;
        logic [2:0]     funct;
        logic [3:0]     dest_reg;
        logic [3:0]     src_reg;
        logic [15:0]    imm;
    } instr_t;

    typedef struct packed {
        logic [15:0]    addr;
        logic [15:0]    data2mem;
        logic           write;
        logic [31:0]    ptr;
        logic           ptr_valid;
        logic           addr_valid;
    } cpu_mem_bus_t;

endpackage
