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
    logic clk_counter = 0;
    always_ff@(posedge clk) begin
        clk_counter <= clk_counter + 1;
        if (clk_counter == 1) begin
            core_clk <= ~core_clk;
        end
    end
    logic core_clk=0;



    initial begin
        $dumpfile("waveforms/top.fst");
        $dumpvars(0, top);
    end

endmodule
