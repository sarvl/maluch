module io_controller (
    input logic        clk,
    input logic        _reset,
    // IO ports to cpu decoder
    input logic        io_w_en,
    input logic [15:0] io_data_w,
    output logic       io_irq,
    output logic       io_busy,

    // Outputs to other parts of GPU
    output logic [15:0] color_data,
    output mode  //0 - font_color | 1 - background_color
);
  // Interrupt and busy flag low because gpu doesn't send data to cpu 
  // and execution of all the instructions is one cycle long 
  // But still in here in case of the change of the design
  assign io_busy = 1'b0;
  assign io_irq  = 1'b0;

  logic [7:0] color;
  logic [7:0] background_color;
  logic [7:0] font_color;
  logic [7:0] instruction;

  assign instruction = io_data_w[15:8];  //Higher 8 bits of instruction
  assign color       = io_data_w[7:0];  //Lower 8 bits of instruction

  assign mode        = instruction[0];

  always_ff @(posedge clk) begin : color_register  //demux
    if (!_reset) begin
      font_color <= 8'hFF;
      background_color <= 0;
    end else if (io_w_en && instruction[1]) begin
      if (~mode) font_color <= color;
      else background_color <= color;
    end
  end : color_register
  assign color_data = {font_color, background_color};
endmodule : io_controller
