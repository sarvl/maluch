module alu(
    input logic [15:0] src1, src2,
    input logic [2:0] aopcode,
    output logic [15:0] result
);

    always_comb begin
        case (aopcode)
            3'b000: result = src1 + src2;
            3'b001: result = src1 - src2;
            3'b010: result = src1 & src2;
            3'b011: result = src1 | src2;
            3'b100: result = src1 ^ src2;
            3'b101: result = ~src2;
            3'b110: result = src2 >> 1;
            3'b111: result = src2 << 1;
        endcase
    end

endmodule
