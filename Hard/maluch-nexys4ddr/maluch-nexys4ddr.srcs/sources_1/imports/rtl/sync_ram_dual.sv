`timescale 1ns / 1ps
module sync_ram_dual #(
    parameter int AddrSize=16,
    parameter int DataSize=16
)(
    input logic [AddrSize-1:0] pa_addr_i,
    output logic [DataSize-1:0] pa_data_o,
    input logic [AddrSize-1:0] pb_addr_i,
    output logic [DataSize-1:0] pb_data_o,
    input logic [AddrSize-1:0] w_addr_i,
    input logic [DataSize-1:0] w_data_i,
    input logic w_en,
    input logic clk
);
    (* ram_style = "block" *)
    logic [DataSize-1:0] block [2**AddrSize];
    
    initial begin
        for (int i=0; i<2**AddrSize; i++) begin
            block[i] = 16'hFF20; // White-on-Black Spaces
        end
    end

    always_ff @(posedge clk) begin
        if (w_en) block[w_addr_i] <= w_data_i;
        pa_data_o <= block[pa_addr_i];
        pb_data_o <= block[pb_addr_i];
    end
endmodule
