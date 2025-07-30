`include "types.sv"

import Types::*;

module decoder (
    input logic clk,
    input logic _reset,
    input logic [31:0] instr_in,

    output instr_t code_bus,
    output logic [15:0] imm,
    output logic [3:0] dest_reg, src_reg
);

    logic [31:0] instr;
    always_comb instr = _reset ? 0 : instr_in;
    always_ff @(posedge clk) code_bus.opcode <= instr[15+16:12+16];
    always_ff @(posedge clk) code_bus.funct <= instr[10+16:8+16];
    always_ff @(posedge clk) code_bus.imm_present <= instr[11+16];
    always_ff @(posedge clk) dest_reg <= instr[7+16:4+16];
    always_ff @(posedge clk) src_reg <= instr[3+16:16];
    always_ff @(posedge clk) imm <= instr[15:0];

    initial begin
        $dumpvars(0, decoder);
    end

endmodule
