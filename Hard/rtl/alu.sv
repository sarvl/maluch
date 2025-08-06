`include "types.sv"
`include "core.sv"

import types::csr_t;

module alu(
    core.ALU CoreBus
);

    logic [15:0] src1, src2, result;
    assign src1 = CoreBus.src1;
    assign src2 = CoreBus.src2;

    logic carry;
    pulldown(carry);

    always_comb begin
        unique case (CoreBus.alu_ctrl)
            3'b000: {carry, result} = src1 + src2;
            3'b001: {carry, result} = src1 - src2;
            3'b010: result = src1 & src2;
            3'b011: result = src1 | src2;
            3'b100: result = src1 ^ src2;
            3'b101: result = ~src2;
            3'b110: result = src2 >> 1;
            3'b111: result = src2 << 1;
        endcase

        assign CoreBus._csr_next.Sign = result[15];
        assign CoreBus._csr_next.Zero = (result == 16'h0000);
        assign CoreBus._csr_next.Carry = carry;
        assign CoreBus._csr_next.Overflow = (src1[15] == src2[15]) && (result[15] != src1[15]);
    end

    assign CoreBus.alu_ret = result;

    initial begin
        $dumpfile("waveforms/alu.fst");
        $dumpvars(0, alu);
    end

endmodule
