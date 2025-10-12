`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif
`ifndef CORE
    `include "core_i.sv"
    `define CORE
`endif

import types::csr_t;

module counter (
    `ifndef PRECISE_SIM
        core_i.IP CoreBus
    `else
        input logic [31:0]  instr_pointer,
        input csr_t         csr,
        input logic [31:0]  instruction,
        input logic [15:0]  src2,
        output logic [31:0] _next_pointer
    `endif
);

    import types::instr_t;

    logic branch_valid;
    `ifndef PRECISE_SIM
        csr_t csr;
        assign csr = CoreBus.csr;
    `endif

    logic [3:0] _code;
    `ifndef PRECISE_SIM 
        assign _code = {CoreBus.instruction[28], CoreBus.instruction[26:24]};
    `else
        assign _code = {instruction[28], instruction[26:24]};
    `endif

    logic branching;
    `ifndef PRECISE_SIM
        assign branching = (CoreBus.instruction[31:28] ==? 4'b010x) ? 1 : 0;
    `else
        assign branching = (instruction[31:28] ==? 4'b010x) ? 1 : 0;
    `endif

    `ifndef PRECISE_SIM
        logic [31:0] instr_pointer;
        assign instr_pointer = CoreBus.instr_pointer;
    `endif

    `ifndef PRECISE_SIM
        logic [15:0] src2;
        assign src2 = CoreBus.src2;
    `endif



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


    instr_t i;
    `ifndef PRECISE_SIM
        assign i = CoreBus.instruction;
    `else
        assign i = instruction;
    `endif

    logic [15:0]    _pointer0;
    logic [15:0]    _pointer1;

    always_comb _pointer0 = branching && branch_valid ? src2 :
                            i.imm_valid ? instr_pointer[15:0] + 1 :
                            instr_pointer[15:0];

    always_comb _pointer1 = branching && branch_valid ? src2 + 1 :
                            i.imm_valid ? instr_pointer[15:0] + 2 :
                            instr_pointer[15:0] + 1;


    `ifndef PRECISE_SIM
        assign CoreBus._next_pointer[31:16] = _pointer0;
        assign CoreBus._next_pointer[15:0] = _pointer1;
    `else
        assign _next_pointer = {_pointer0, _pointer1};
    `endif

endmodule
