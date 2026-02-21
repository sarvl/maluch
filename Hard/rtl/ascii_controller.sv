`include "char_rom.sv"

module ascii_controller (
    input  logic [15:0] color_data,
    input  logic [ 7:0] vram_data,
    input  logic [19:0] address,
    output logic [11:0] ascii_address,
    output logic [ 7:0] data_ascii
);
  localparam int CHAR_WIDTH = 8;
  localparam int CHAR_HEIGHT = 16;

  logic [7:0] font_color;
  logic [7:0] background_color;
  assign font_color       = color_data[15:8];  //Higher 8 bits of color_data
  assign background_color = color_data[7:0];  //Lower 8 bits of color_data

  logic [9:0] pixel_x;
  logic [9:0] pixel_x_prefetch;
  logic [9:0] pixel_y;
  assign pixel_x = address[9:0];
  assign pixel_y = address[19:10];

  assign pixel_x_prefetch = pixel_x + 10'd1;
  logic [4:0] row;
  logic [6:0] column;
  assign row = pixel_y[8:4];
  assign column = pixel_x_prefetch[9:3];

  logic [11:0] char_address;
  logic [ 7:0] data_rom;

  char_rom char_rom (
      .char_address(char_address),
      .data_rom(data_rom)
  );

  logic [2:0] index;
  always_comb begin
    index = CHAR_WIDTH - {1'b0, pixel_x[2:0]} - 1'b1;
    data_ascii = (data_rom[index] === 1'b1) ? font_color : background_color;
    char_address = vram_data * CHAR_HEIGHT + {8'b0, pixel_y[3:0]};
    ascii_address = {row, column};
  end
endmodule : ascii_controller
