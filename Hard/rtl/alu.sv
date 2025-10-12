`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif

`ifndef CORE
    `include "core_i.sv"
    `define CORE
`endif



module alu(
    core_i.ALU CoreBus
);

    import types::csr_t;

    logic [15:0] src1, src2, result;
    assign src1 = CoreBus.src1;
    assign src2 = CoreBus.src2;

    logic carry;

    always_comb begin
        unique case (CoreBus.alu_ctrl)
            3'b000: {carry, result} = src1 + src2;
            3'b001: {carry, result} = src1 - src2;
            3'b010: begin result = src1 & src2; carry = 0; end
            3'b011: begin result = src1 | src2; carry = 0; end
            3'b100: begin result = src1 ^ src2; carry = 0; end
            3'b101: begin result = ~src2; carry = 0; end
            3'b110: begin result = src2 >> 1; carry = 0; end
            3'b111: begin result = src2 << 1; carry = 0; end
        endcase

        assign CoreBus._csr_next.Sign = result[15];
        assign CoreBus._csr_next.Zero = (result == 16'h0000);
        assign CoreBus._csr_next.Carry = carry;
        assign CoreBus._csr_next.Overflow = (src1[15] == src2[15]) && (result[15] != src1[15]);
    end

    assign CoreBus.alu_ret = result;


endmodule
