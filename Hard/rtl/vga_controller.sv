module vga_controller #(
    // Configuration for the 640x480 resolution 60Hz refresh rate and 25.175MHz clock
    // Period of 39.72 nanoseconds (ns)
    // Horizontal timings [pixels]
    parameter H_VISIBLE_AREA = 640,
    parameter H_FRONT_PORCH  = 16,
    parameter H_SYNC_PULSE   = 96,
    parameter H_BACK_PORCH   = 48,
    parameter H_WHOLE_LINE   = H_SYNC_PULSE + H_BACK_PORCH + H_VISIBLE_AREA + H_FRONT_PORCH,  //800
    // Vertical timings [lines]
    parameter V_VISIBLE_AREA = 480,
    parameter V_FRONT_PORCH  = 10,
    parameter V_SYNC_PULSE   = 2,
    parameter V_BACK_PORCH   = 33,
    parameter V_WHOLE_FRAME  = V_SYNC_PULSE + V_BACK_PORCH + V_VISIBLE_AREA + V_FRONT_PORCH   //525
) (
    input logic clk,
    input logic rst,
    // 256 colors mode
    input logic [7:0] data,
    output logic v_sync,
    output logic h_sync,
    output logic [2:0] red,
    output logic [2:0] green,
    output logic [1:0] blue,
    //Pixel address, 10 lower bits is x-axis and 10 upper bits is y-axis
    output logic [19:0] address,
    output logic video_enable
);
  //logic video_enable;
  logic done_line;
  logic done_frame;

  // Horizontal and vertical counter registers
  logic [10:0] x;
  logic [9:0] y;

  // Pixel registers (visible_area)
  logic [10:0] pixel_x;
  logic [9:0] pixel_y;

  // Counter logic
  always_ff @(posedge clk or posedge rst) begin : h_counter
    if (rst || done_line) x <= 11'b0;
    else x <= x + 1;
  end : h_counter

  always_ff @(posedge clk or posedge rst) begin : v_counter
    if (rst || done_frame) y <= 10'b0;
    else if (done_line) y <= y + 1;
  end : v_counter

  // Comparators
  assign h_sync = ~(x < H_SYNC_PULSE);
  assign v_sync = ~(y < V_SYNC_PULSE);
  assign done_line = (x >= H_WHOLE_LINE - 1);
  assign done_frame = (y >= V_WHOLE_FRAME - 1);
  assign video_enable = (
    x >= (H_SYNC_PULSE + H_FRONT_PORCH) &&
    x < (H_SYNC_PULSE + H_FRONT_PORCH + H_VISIBLE_AREA)
    &&
    y >= (V_SYNC_PULSE + V_FRONT_PORCH) &&
    y < (V_SYNC_PULSE + V_FRONT_PORCH + V_VISIBLE_AREA)
  );

  //Calculate visible counter if ~video_enable then pixel_x and pixel_y is garbage
  assign pixel_x = x - (H_SYNC_PULSE + H_FRONT_PORCH);  //Max 10 bits needed for addressing
  assign pixel_y = y - (V_SYNC_PULSE + V_FRONT_PORCH);  //Max 10 bits needed for addressing

  assign address = {pixel_y, pixel_x[9:0]};

  // RGB data registers
  always_ff @(posedge clk or posedge rst) begin : rgb_register
    if (rst || !video_enable || data[0] === 1'bx) begin
      red   <= 3'b0;
      green <= 3'b0;
      blue  <= 2'b0;
    end else begin
      red   <= data[7:5];
      green <= data[4:2];
      blue  <= data[1:0];
    end
  end : rgb_register
endmodule  //vga
