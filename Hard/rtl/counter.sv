`include "types.sv"
`include "core.sv"

import types::csr_t;
import types::instr_t;

module counter (
    core.IP CoreBus
);

    logic branch_valid;
    csr_t csr;
    assign csr = CoreBus.csr;

    logic [3:0] _code;
    assign _code = {CoreBus.instruction[28], CoreBus.instruction[26:24]};

    logic branching;
    assign branching = (CoreBus.instruction[31:28] ==? 4'b010x) ? 1 : 0;

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
    assign i = CoreBus.instruction;

    logic [15:0]    _pointer0;
    logic [15:0]    _pointer1;

    always_comb _pointer0 = branching && branch_valid ? CoreBus.src2 :
                            i.imm_valid ? CoreBus.instr_pointer[15:0] + 1 :
                            CoreBus.instr_pointer[15:0];

    always_comb _pointer1 = branching && branch_valid ? CoreBus.src2 + 1 :
                            i.imm_valid ? CoreBus.instr_pointer[15:0] + 2 :
                            CoreBus.instr_pointer[15:0] + 1;

    assign CoreBus._next_pointer[31:16] = _pointer0;
    assign CoreBus._next_pointer[15:0] = _pointer1;

endmodule
