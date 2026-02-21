`include "gpu_controller.sv"
`include "vga_controller.sv"
`include "vram.sv"

module graphics_card (
    input  logic        clk,
    input  logic        _reset,
    output logic [ 2:0] red,
    output logic [ 2:0] green,
    output logic [ 1:0] blue,
    output logic        h_sync,
    output logic        v_sync,
    output logic        video_enable,
    // IO ports to cpu decoder (according to interface.pdf)
    input  logic [ 2:0] io2gpu_addr,
    input  logic        io2gpu_w_en,
    input  logic [15:0] io2gpu_data_w,
    // To VRAM (read-only)
    input  logic [ 7:0] mem2gpu_data,
    output logic [15:0] gpu2mem_addr
);
  logic [19:0] address;
  logic [ 7:0] data_out;

  gpu_controller gpu_controller (
      .clk(clk),
      ._reset(_reset),
      // IO ports to cpu decoder
      .io_addr(io2gpu_addr),
      .io_w_en(io2gpu_w_en),
      .io_data_w(io2gpu_data_w),
      //Other connections to GPU
      .v_sync(v_sync),
      .address(address),
      .data_out(data_out),
      .vram_addr(gpu2mem_addr),
      .vram_data(mem2gpu_data),
  );

  vga_controller vga_controller (
      .clk(clk),
      ._reset(_reset),
      .address(address),
      .v_sync(v_sync),
      .h_sync(h_sync),
      .data(data_out),
      .red(red),
      .green(green),
      .blue(blue),
      .video_enable(video_enable)
  );
endmodule : graphics_card
