`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif
//`include "core.sv"

module top (
    input logic clk,
    input logic _reset,
    input logic [31:0] instr_in,

    output logic [31:0] pointer
);

    // -----------------------------------
    //  Clocking
    // -----------------------------------
    logic clk_core = 0;
    always_ff@(posedge clk) begin
        clk_core <= clk_core + 1;
    end



    initial begin
        $dumpfile("waveforms/top.fst");
        $dumpvars(0, top);
    end

endmodule
