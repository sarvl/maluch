`include "io_controller.sv"
`include "ascii_controller.sv"
`include "clear_engine.sv"

module gpu_controller (
    input  logic        clk,
    input  logic        _reset,
    input  logic [19:0] address,
    input  logic        v_sync,
    input  logic [ 7:0] vram_data,  //ascii code
    output logic [15:0] vram_addr,
    output logic [ 7:0] data_out,

    // IO ports to cpu decoder
    input logic [ 2:0] io_addr,
    input logic        io_w_en,
    input logic [15:0] io_data_w
);
  logic [15:0] color_data;
  logic mode;

  io_controller io_controller (
      .clk(clk),
      ._reset(_reset),
      // IO ports to cpu decoder
      .io_addr(io_addr),
      .io_w_en(io_w_en),
      .io_data_w(io_data_w),
      // Other connections to GPU
      .color_data(color_data),
      .mode(mode)
  );

  logic [ 7:0] data_ascii;
  logic [11:0] ascii_address;

  ascii_controller ascii_controller (
      .color_data(color_data),
      .vram_data(vram_data),
      .address(address),
      .ascii_address(ascii_address),
      .data_ascii(data_ascii)
  );
  assign data_out = data_ascii;

  // Making sure address will be valid when reading from VRAM on posedge
  always_ff @(negedge clk) begin : vram_addr_ff
    vram_addr <= {1'b1, 3'b0, ascii_address};
  end
endmodule : gpu_controller
