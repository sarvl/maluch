`timescale 1ns / 10ps
`include "graphics_card.sv"

module graphics_card_tb ();
  int          data = 0;
  logic [ 6:0] x;
  logic [ 4:0] y;

  int          fd;
  bit          capturing;
  logic        clk;
  logic        _reset;
  logic        v_sync;
  logic        h_sync;
  logic [ 7:0] instruction;
  logic [ 7:0] color;
  logic [ 7:0] ascii_code;
  logic [ 2:0] red;
  logic [ 2:0] green;
  logic [ 1:0] blue;
  logic        video_enable;

  // VRAM <-> GPU
  logic [ 7:0] mem2gpu_data;
  logic [15:0] gpu2mem_addr;
  // VRAM <-> CPU (for testbenching purposes)
  logic [ 7:0] vram_write_data;

  // IO ports to cpu decoder
  logic [ 2:0] io2gpu_addr;
  logic        io2gpu_w_en;
  logic [15:0] io2gpu_data_w;
  assign io2gpu_data_w = {instruction, color};

  graphics_card graphics_card (
      .clk(clk),
      ._reset(_reset),
      // IO ports to cpu decoder
      .io2gpu_addr(io2gpu_addr),
      .io2gpu_w_en(io2gpu_w_en),
      .io2gpu_data_w(io2gpu_data_w),
      // Other connections
      .h_sync(h_sync),
      .v_sync(v_sync),
      .red(red),
      .green(green),
      .blue(blue),
      .video_enable(video_enable),
      // To VRAM (read-only)
      .mem2gpu_data(mem2gpu_data),
      .gpu2mem_addr(gpu2mem_addr)
  );

  vram vram (
      .clk(clk),
      .vram_addr(gpu2mem_addr),
      .vram_write_data(vram_write_data),
      .mem2gpu_data(mem2gpu_data)
  );

  //Clock
  always #14 clk = ~clk;  //~35Mhz pixel_clock

  //TASKS:
  //wait N frames
  task automatic wait_frames(int n);
    repeat (n) @(negedge v_sync);
  endtask

  task automatic capture_one_frame(string path);
    fd = $fopen(path, "w");
    @(negedge v_sync);  // start of frame
    capturing = 1;
    @(negedge v_sync);  // next frame edge
    capturing = 0;
    $fclose(fd);
  endtask

  // CPU tasks
  task automatic write_char(int row, int col, byte ascii_code);
    @(posedge clk);
    #7;
    gpu2mem_addr = {1'b0, 3'b000, row[4:0], col[6:0]};  // Address, bit15=0 => write
    vram_write_data = ascii_code;
    @(negedge clk);
    #7;
    gpu2mem_addr = 'bZ;  // Simulate HIGH-Z
  endtask

  task automatic set_font_color(byte c);
    color = c;
    instruction = 8'h02;
    @(posedge clk);
    instruction = 8'h00;
  endtask

  task automatic set_bg_color(byte c);
    color = c;
    instruction = 8'h03;
    @(posedge clk);
    instruction = 8'h00;
  endtask

  initial begin
    // Defaults
    clk = 0;
    io2gpu_addr = 3'd2;
    io2gpu_w_en = 1;

    instruction = 8'h00;
    color = 8'h00;

    _reset = 1;

    // Holding reset for proper initialization
    _reset = 0;
    repeat (4) @(posedge clk);
    _reset = 1;

    set_font_color(8'b00011100);  // green font
    set_bg_color(8'h00);  // black background

    //VRAM character write
    for (y = 0; y < 30; y++) begin
      for (x = 0; x < 80; x++) begin
        data++;
        write_char(y, x, data[7:0]);
      end
    end

    capture_one_frame("build/trial");
    $finish;
  end

  // Only one frame in png file
  always_ff @(posedge clk) begin : vga_out
    if (capturing && video_enable) $fdisplay(fd, "%08b", {red, green, blue});
  end : vga_out

  initial begin
    $dumpfile("waveforms/graphics_card_tb.fst");
    $dumpvars(0, graphics_card_tb);
  end
endmodule
