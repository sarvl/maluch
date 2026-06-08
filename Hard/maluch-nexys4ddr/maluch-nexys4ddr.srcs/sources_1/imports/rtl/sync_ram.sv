`timescale 1ns / 1ps
module sync_ram #(
    parameter int AddrSize=16,
    parameter int DataSize=16,
    parameter INIT_FILE = ""
)(
    input  logic                 clk,
    // Port A (Read-Only)
    input  logic [AddrSize-1:0]  pa_addr_i,
    output logic [DataSize-1:0]  pa_data_o,
    // Port B (Read/Write)
    input  logic [AddrSize-1:0]  pb_addr_i,
    output logic [DataSize-1:0]  pb_data_o,
    input  logic [DataSize-1:0]  w_data_i,
    input  logic                 w_en
);
    (* ram_style = "block" *)
    logic [DataSize-1:0] block [2**AddrSize];
    
    initial begin
        if (INIT_FILE != "") begin
            $readmemh(INIT_FILE, block);
        end else begin
            for (int i=0; i<2**AddrSize; i++) block[i] = '0;
        end
    end
    
    always_ff @(posedge clk) begin
        pa_data_o <= block[pa_addr_i];
    end

    always_ff @(posedge clk) begin
        if (w_en) block[pb_addr_i] <= w_data_i;
        pb_data_o <= block[pb_addr_i];
    end
endmodule
