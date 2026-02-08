`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif

import types::csr_t;

module counter (
        input logic [15:0]  instr_pointer,
        input csr_t         csr,
        input logic [31:0]  instruction,
        input logic [15:0]  instr_pointer_ctrl,
        
        output logic [15:0] _nxt_instr_pointer
);

    import types::instr_t;
    instr_t i;
    assign i = instruction;

    logic branch_valid;

    logic [3:0] _code;
    assign _code = {i.opcode[0], i.funct};

    logic branching;
    assign branching = (i.opcode ==? 4'b010x) ? 1 : 0;


    always_comb begin

        unique case (_code)
            4'b0000: branch_valid = 1;
            4'b0001: branch_valid = csr.Zero;
            4'b0010: branch_valid = ~csr.Zero;
            4'b0011: branch_valid = csr.Sign ~^ csr.Overflow;
            4'b0100: branch_valid = csr.Zero & (csr.Sign ^ csr.Overflow);
            4'b0101: branch_valid = ~csr.Zero & (csr.Sign ~^ csr.Overflow);
            4'b0110: branch_valid = csr.Sign ^ csr.Overflow;
            4'b0111: branch_valid = csr.Overflow;
            4'b1000: branch_valid = 0; // branch if busy
            4'b1001: branch_valid = csr.Sign;
            4'b1010: branch_valid = ~csr.Sign;
            4'b1011: branch_valid = ~csr.Carry;
            4'b1100: branch_valid = csr.Zero | csr.Carry;
            4'b1101: branch_valid = ~(csr.Zero | csr.Carry);
            4'b1110: branch_valid = csr.Carry;
            4'b1111: branch_valid = ~csr.Overflow;
        endcase

    end

    logic [15:0]    _pointer;

    always_comb _pointer = branching && branch_valid ?
                            instr_pointer_ctrl : instr_pointer + 2;


    assign _nxt_instr_pointer = _pointer;

endmodule
