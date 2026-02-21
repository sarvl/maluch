module clear_engine (
    input  logic        clk,
    input  logic        _reset,
    input  logic        start,      // 1-cycle pulse
    input  logic        line_mode,  // 1=line, 0=full
    input  logic [ 4:0] line_indx,  // valid when line_mode=1
    output logic        busy,
    output logic        done,       // 1-cycle pulse
    output logic [15:0] waddr,      // bit15=0 => write enabled in VRAM
    output logic [ 7:0] wdata
);
  localparam int COLS = 80;
  localparam int ROWS = 30;
  localparam int CLEAR_VALUE = 8'h20;  //space

  logic [6:0] x;
  logic [4:0] y;
  logic       mode_latched;
  logic [4:0] line_latched;

  always_ff @(posedge clk or posedge _reset) begin : clear_logic
    if (!_reset) begin
      busy <= 0;
      done <= 0;
      x <= '0;
      y <= '0;
      mode_latched <= 0;
      line_latched <= '0;
      waddr <= '0;
      wdata <= CLEAR_VALUE;
    end else begin
      done <= 0;
      if (!busy) begin
        waddr <= 16'h8000;
        if (start) begin
          mode_latched <= line_mode;
          line_latched <= line_indx;
          x <= 0;
          y <= line_mode ? line_indx : 0;
          busy <= 1;
        end
      end else begin
        wdata <= CLEAR_VALUE;
        waddr <= {1'b0, 3'b000, y, x};  // bit15=0 -> write

        if (x == COLS) begin
          x <= 0;
          if (mode_latched) begin  //if line_mode
            busy <= 0;
            done <= 1;
          end else if (y == ROWS - 1) begin
            busy <= 0;
            done <= 1;
          end else begin
            y <= y + 1;
          end
        end else begin
          x <= x + 1;
        end
      end
    end
  end : clear_logic
endmodule : clear_engine
