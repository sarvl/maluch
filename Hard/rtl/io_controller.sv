module io_controller (
    input logic        clk,
    input logic        _reset,
    // IO ports to cpu decoder
    input logic [ 2:0] io_addr,
    input logic        io_w_en,
    input logic [15:0] io_data_w,

    // Outputs to other parts of GPU
    output logic [15:0] color_data,
    output mode  //0 - font_color | 1 - background_color
);
  logic [7:0] color;
  logic [7:0] background_color;
  logic [7:0] font_color;
  logic [7:0] instruction;

  // Address handling from decoder
  localparam logic [2:0] GPU_ADDR = 3'b010;  //GPU IO address
  logic sel;
  assign sel         = (io_addr == GPU_ADDR);

  assign instruction = io_data_w[15:8];  //Higher 8 bits of instruction
  assign color       = io_data_w[7:0];  //Lower 8 bits of instruction

  assign mode        = instruction[0];

  always_ff @(posedge clk) begin : color_register  //demux
    if (!_reset) begin
      font_color <= 8'hFF;
      background_color <= 0;
    end else if (sel && io_w_en && instruction[1]) begin
      if (~mode) font_color <= color;
      else background_color <= color;
    end
  end : color_register
  assign color_data = {font_color, background_color};
endmodule : io_controller
