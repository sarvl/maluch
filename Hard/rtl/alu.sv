`include "types.sv"

import Types::*;

module alu(
    input logic [15:0] src1, src2,
    input logic [2:0] aopcode,
    output logic [15:0] result,
    output csr_t csr
);

    logic carry;
    pulldown(carry);

    always_comb begin
        case (aopcode)
            3'b000: {carry, result} = src1 + src2;
            3'b001: {carry, result} = src1 - src2;
            3'b010: result = src1 & src2;
            3'b011: result = src1 | src2;
            3'b100: result = src1 ^ src2;
            3'b101: result = ~src2;
            3'b110: result = src2 >> 1;
            3'b111: result = src2 << 1;
        endcase

        assign csr.Sign = result[15];
        assign csr.Zero = (result == 16'h0000);
        assign csr.Carry = carry;
        assign csr.Overflow = (src1[15] == src2[15]) && (result[15] != src1[15]);
    end

    initial begin
        $dumpfile("waveforms/alu.fst");
        $dumpvars(0, alu);
    end

endmodule
