module vga_controller (
    input logic clk,
    input logic _reset,
    input logic [7:0] data,
    output logic v_sync,
    output logic h_sync,
    output logic [2:0] red,
    output logic [2:0] green,
    output logic [1:0] blue,
    output logic [19:0] address,
    output logic video_enable
);
  localparam int H_VISIBLE = 640;
  localparam int H_FP      = 16;
  localparam int H_SYNC    = 96;
  localparam int H_BP      = 48;
  localparam int H_TOTAL   = 800;
  localparam int V_VISIBLE = 480;
  localparam int V_FP      = 10;
  localparam int V_SYNC    = 2;
  localparam int V_BP      = 33;
  localparam int V_TOTAL   = 525;

  logic [9:0] x, y;
  always_ff @(posedge clk) begin
    if (_reset) begin x <= 0; y <= 0; end
    else if (x == H_TOTAL - 1) begin
      x <= 0;
      if (y == V_TOTAL - 1) y <= 0; else y <= y + 1;
    end else x <= x + 1;
  end

  // 3-Stage Pipeline to match registered GPU address + BRAM read
  logic h_sync_pipe[3], v_sync_pipe[3], video_en_pipe[3];
  always_ff @(posedge clk) begin
    h_sync_pipe[0]   <= ~(x >= (H_VISIBLE + H_FP) && x < (H_VISIBLE + H_FP + H_SYNC));
    v_sync_pipe[0]   <= ~(y >= (V_VISIBLE + V_FP) && y < (V_VISIBLE + V_FP + V_SYNC));
    video_en_pipe[0] <= (x < H_VISIBLE) && (y < V_VISIBLE);
    for (int i = 1; i < 3; i++) begin
      h_sync_pipe[i]   <= h_sync_pipe[i-1];
      v_sync_pipe[i]   <= v_sync_pipe[i-1];
      video_en_pipe[i] <= video_en_pipe[i-1];
    end
  end

  assign h_sync = h_sync_pipe[2];
  assign v_sync = v_sync_pipe[2];
  assign video_enable = video_en_pipe[2];
  assign address = {y, x};

  always_ff @(posedge clk) begin : rgb_register
    if (_reset || !video_en_pipe[2]) begin red <= 0; green <= 0; blue <= 0; end
    else begin
      red <= data[7:5]; green <= data[4:2]; blue <= data[1:0];
    end
  end
endmodule
