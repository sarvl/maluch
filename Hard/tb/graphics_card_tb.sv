`timescale 1ns / 10ps
`include "graphics_card.sv"

module graphics_card_tb ();
  int fd;
  logic clk;
  logic rst;
  logic v_sync;
  logic h_sync;
  logic [7:0] ascii_code;
  logic [2:0] red;
  logic [2:0] green;
  logic [1:0] blue;
  logic video_enable;

  graphics_card graphics_card (
      .clk(clk),
      .rst(rst),
      .h_sync(h_sync),
      .v_sync(v_sync),
      .red(red),
      .green(green),
      .blue(blue),
      .video_enable(video_enable)
  );

  initial begin
    fd  = $fopen("build/trial", "w");
    clk = 0;
    rst = 0;
    #1;
    rst = 1;
    #1;
    rst = 0;
    #20000000;
    $fclose(fd);
    $finish;
  end
  always #14 clk = ~clk;

  always_ff @(posedge clk) begin : vga_out
    if (video_enable) $fdisplay(fd, "%08b", {red, green, blue});
  end : vga_out

  initial begin
    $dumpfile("waveforms/graphics_card_tb.fst");
    $dumpvars(0, graphics_card_tb);
  end
endmodule
