`timescale 1ns / 1ps
module io_controller (
    input logic        clk,
    input logic        _reset,
    input logic        io_w_en,
    input logic [15:0] io_data_w,
    output logic       io_irq,
    output logic       io_busy,
    output logic [15:0] color_data,
    output mode
);
  assign io_busy = 1'b0;
  assign io_irq  = 1'b0;

  logic [7:0] color;
  logic [7:0] background_color = 8'h00;
  logic [7:0] font_color = 8'hFF;
  logic [7:0] instruction;

  assign instruction = io_data_w[15:8];
  assign color       = io_data_w[7:0];
  assign mode        = instruction[0];

  always_ff @(posedge clk) begin : color_register
    if (_reset) begin
      font_color <= 8'hFF;
      background_color <= 8'h00;
    end else if (io_w_en && instruction[1]) begin
      if (~mode) font_color <= color;
      else background_color <= color;
    end
  end
  assign color_data = {font_color, background_color};
endmodule
