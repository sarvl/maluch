module gpu_controller #(
    parameter CHAR_WIDTH = 4'd8,
    parameter CHAR_HEIGHT = 5'd16,
    parameter FONT_COLOR = 8'hFF,
    parameter BACKGROUND_COLOR = 8'h00
) (
    input  logic [19:0] address,
    input  logic [ 7:0] data_in,       //ascii code
    output logic [11:0] vram_address,
    output logic [ 7:0] data_out
);
  logic [ 7:0] data_ascii;
  logic [11:0] ascii_address;

  ascii_controller ascii_controller (
      .data_in(data_in),
      .address(address),
      .ascii_address(ascii_address),
      .data_ascii(data_ascii)
  );
  assign data_out = data_ascii;
  assign vram_address = ascii_address;
endmodule  //gpu_controller

module ascii_controller #(
    parameter CHAR_WIDTH = 4'd8,
    parameter CHAR_HEIGHT = 5'd16,
    parameter FONT_COLOR = 8'hFF,
    parameter BACKGROUND_COLOR = 8'h00,
    parameter DISPLAY_CHAR_WIDTH = 80,
    parameter DISPLAY_CHAR_HEIGHT = 30
) (
    input  logic [ 7:0] data_in,
    input  logic [19:0] address,
    output logic [11:0] ascii_address,
    output logic [ 7:0] data_ascii
);
  logic [9:0] pixel_x;
  logic [9:0] pixel_y;
  assign pixel_x = address[9:0];
  assign pixel_y = address[19:10];

  logic [11:0] char_address;
  logic [ 7:0] data_rom;

  char_rom char_rom (
      .char_address(char_address),
      .data_rom(data_rom)
  );

  logic [2:0] index;
  always_comb begin
    index = CHAR_WIDTH - (pixel_x % CHAR_WIDTH) - 1;
    data_ascii = (data_rom[index] === 1'b1) ? FONT_COLOR : BACKGROUND_COLOR;
    char_address = data_in * CHAR_HEIGHT + pixel_y % CHAR_HEIGHT;
    ascii_address = pixel_y[9:4] * DISPLAY_CHAR_WIDTH + pixel_x[9:3];
  end
endmodule  //ascii_controller

module char_rom (
    input  logic [11:0] char_address,
    output logic [ 7:0] data_rom
);
  // 256 characters × 16 rows = 4096 bytes
  logic [7:0] font_mem[4096];

  // Load font from hex file
  initial begin
    $readmemh("tb/char_font.hex", font_mem);
  end

  always_comb begin
    data_rom = font_mem[char_address];
  end
endmodule  //char_rom
