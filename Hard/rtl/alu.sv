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
    logic carry_msb;

    always_comb begin
        unique case (alu_ctrl)
            3'b000: begin
                {carry_msb, result[14:0]} = src1[14:0] + src2[14:0];
                {carry, result[15]} = src1[15] + src2[15] + carry_msb;
            end
            3'b001: begin
                logic [15:0] _nsrc2 = ~src2;
                {carry_msb, result[14:0]} = src1[14:0] + _nsrc2[14:0] + 1;
                {carry, result[15]} = src1[15] + _nsrc2[15] + carry_msb;
            end
            3'b010: result = src1 & src2;
            3'b011: result = src1 | src2;
            3'b100: result = src1 ^ src2;
            3'b101: result = ~src2;
            3'b110: result = src2 >> 1;
            3'b111: result = src2 << 1;
        endcase

        assign _csr_next.Sign = result[15];
        assign _csr_next.Zero = (result == '0);
        assign _csr_next.Carry = carry;
        assign _csr_next.Overflow = (carry_msb != carry);
    end

    assign alu_ret = result;


endmodule
