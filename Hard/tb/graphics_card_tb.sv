`timescale 1ns / 10ps

module graphics_card_tb ();
  int fd;
  logic clk;
  logic rst;
  logic v_sync;
  logic h_sync;
  logic [7:0] ascii_code;
  logic mode;
  logic [2:0] red;
  logic [2:0] green;
  logic [1:0] blue;
  logic video_enable;

  graphics_card graphics_card (
      .clk(clk),
      .rst(rst),
      .mode(mode),
      .h_sync(h_sync),
      .v_sync(v_sync),
      .red(red),
      .green(green),
      .blue(blue),
      .video_enable(video_enable)  //only for testbenches?
  );

  initial begin
    fd   = $fopen("../tb/trial", "w");
    mode = 0;  //ascii mode
    clk  = 0;
    rst  = 0;
    #1;
    rst = 1;
    #1;
    rst = 0;
    #20ms;
    mode = 1;  //color mode
    #20ms;
    $fclose(fd);
    $finish;
  end
  always #14 clk = ~clk;


  always_ff @(posedge clk) begin : vga_out
    if (video_enable)
      $fdisplay(fd, "%0b", red[0]);  // TODO: add color (rgb) handling in trial2png.py file
  end

  // Optional simple monitor to confirm DUT is alive
  initial begin
    $timeformat(-9, 1, " ns", 8);
    #150;
    $display("[%0t] monitor: v_sync=%b h_sync=%b red=%0h green=%0h blue=%0h", $time, v_sync,
             h_sync, red, green, blue);
  end

  initial begin
    $dumpfile("../tb/graphics_card_tb.vcd");
    $dumpvars(0, graphics_card_tb);
  end
endmodule  //vga_tb
