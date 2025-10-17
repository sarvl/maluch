module vram #(
    parameter MEMORY_BYTES = 2400
) (
    input logic rst,
    input logic clk,
    input logic [11:0] vram_address,
    input logic w_enable,
    input logic [7:0] w_data,
    output logic [7:0] r_data
);
  logic [7:0] mem[MEMORY_BYTES];

  // Testbench only
  initial begin
    for (int i = 0; i < 2400; i++) begin
      mem[i] = i[7:0];
    end
  end

  always_ff @(negedge clk) begin : memory_read
    r_data <= mem[vram_address];
  end : memory_read

  always_ff @(posedge clk) begin : memory_write
    if (w_enable) mem[vram_address] <= w_data;
  end : memory_write

  always_ff @(posedge clk) begin : memory_rst
    if (rst) begin
      for (int i = 0; i < MEMORY_BYTES; i++) begin
        mem[i] = 8'b0;
      end
    end
  end : memory_rst
endmodule
