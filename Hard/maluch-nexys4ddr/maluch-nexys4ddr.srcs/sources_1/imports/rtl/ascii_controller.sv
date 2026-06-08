`timescale 1ns / 1ps
`include "char_rom.sv"

module ascii_controller (
    input  logic        clk,
    input  logic        _reset,
    input  logic [15:0] color_data,
    input  logic [15:0] vram_data,
    input  logic [19:0] address,
    output logic [12:0] ascii_address,
    output logic [ 7:0] data_ascii
);
  localparam int CHAR_WIDTH = 8;
  localparam int CHAR_HEIGHT = 16;

  logic [9:0] pixel_x, pixel_y;
  assign pixel_x = address[9:0];
  assign pixel_y = address[19:10];

  // Register the VRAM address to break timing paths (Stage 1)
  always_ff @(posedge clk) begin
    ascii_address <= {pixel_y[8:4], 8'h0} | {5'b0, pixel_x[9:3]};
  end

  // Delay the pixel sub-coordinates by 2 cycles to match VRAM (1) + Char ROM (1) latency
  logic [2:0] px_d1, px_d2;
  logic [3:0] py_d1, py_d2;
  always_ff @(posedge clk) begin
    px_d1 <= pixel_x[2:0];
    py_d1 <= pixel_y[3:0];
    px_d2 <= px_d1;
    py_d2 <= py_d1;
  end

  logic [11:0] char_rom_addr;
  logic [ 7:0] char_rom_data;

  char_rom char_rom_inst (
      .char_address(char_rom_addr),
      .data_rom(char_rom_data)
  );

  always_comb begin
    char_rom_addr = vram_data[7:0] * CHAR_HEIGHT + {8'b0, py_d2};
    data_ascii = char_rom_data[3'd7 - px_d2] ? 
                 ((vram_data[15:8] != 8'h00) ? vram_data[15:8] : color_data[15:8]) : 
                 color_data[7:0];
  end

endmodule
