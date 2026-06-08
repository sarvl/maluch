module char_rom (
    input  logic [11:0] char_address,
    output logic [ 7:0] data_rom
);
  localparam int CHAR_HEIGHT = 16;
  localparam int NUM_OF_CHARS = 256;
  localparam int MEMORY_BYTES = CHAR_HEIGHT * NUM_OF_CHARS;

  logic [7:0] font_mem[MEMORY_BYTES];

  initial begin
    $readmemh("char_font.hex", font_mem);
  end

  assign data_rom = font_mem[char_address];
endmodule : char_rom
