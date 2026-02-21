module vram (
    input logic clk,
    input logic [ADDR_SIZE-1:0] vram_addr,
    input logic [7:0] vram_write_data,
    output logic [7:0] mem2gpu_data
);
  localparam int ADDR_SIZE = 16;
  localparam int MEMORY_BYTES = $rtoi($pow(2, ADDR_SIZE));

  logic nw_enable;
  assign nw_enable = vram_addr[15];  //if 0 - write

  logic [7:0] mem[MEMORY_BYTES];

  always_ff @(posedge clk) begin : memory_read
    mem2gpu_data <= mem[{1'b0, vram_addr[14:0]}];
  end : memory_read

  always_ff @(negedge clk) begin : memory_write
    if (~nw_enable) mem[{1'b0, vram_addr[14:0]}] <= vram_write_data;
  end : memory_write

endmodule : vram
