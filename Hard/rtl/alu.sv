`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif

import types::csr_t;

module alu(
    input logic [2:0]     alu_ctrl,
    input logic [15:0]    src1,
    input logic [15:0]    src2,

    output logic [15:0]    alu_ret,
    output csr_t _csr_next
);

    logic [15:0] result;

    logic carry;

    always_comb begin
        unique case (alu_ctrl)
            3'b000: {carry, result} = src1 + src2;
            3'b001: {carry, result} = src1 - src2;
            3'b010: begin result = src1 & src2; carry = 0; end
            3'b011: begin result = src1 | src2; carry = 0; end
            3'b100: begin result = src1 ^ src2; carry = 0; end
            3'b101: begin result = ~src2; carry = 0; end
            3'b110: begin result = src2 >> 1; carry = 0; end
            3'b111: begin result = src2 << 1; carry = 0; end
        endcase

        assign _csr_next.Sign = result[15];
        assign _csr_next.Zero = (result == 16'h0000);
        assign _csr_next.Carry = carry;
        assign _csr_next.Overflow = (src1[15] == src2[15]) && (result[15] != src1[15]);
    end

    assign alu_ret = result;


endmodule
