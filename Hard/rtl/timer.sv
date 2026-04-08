module timer #(
    parameter int unsigned CLK_HZ    = 1_000_000,  //1MHz
    parameter int unsigned PERIOD_US = 1000        //1 ms = 1000 us
) (
    input logic clk,
    input logic _reset,

    // io2tim_r_en should be pulsed high for clk cycle when the interrupt is serviced
    input  logic io2tim_r_en,
    output logic tim2io_int_f,
    output logic tim2io_busy_f
);

  // number of clock cycles in the period
  localparam int CYCLES_PER_US = CLK_HZ / 1_000_000;
  localparam int PERIOD_CYCLES = PERIOD_US * CYCLES_PER_US;

  logic [$clog2(PERIOD_CYCLES)-1:0] count;

  always_ff @(posedge clk) begin
    if (!_reset) begin
      count         <= '0;
      tim2io_int_f  <= 1'b0;
      tim2io_busy_f <= 1'b0;
    end else begin
      if (tim2io_int_f) begin
        if (io2tim_r_en) begin  // clear interrupt
          tim2io_int_f  <= 1'b0;
          tim2io_busy_f <= 1'b0;
          count         <= '0;
        end
      end else begin
        if (count == PERIOD_CYCLES - 1) begin
          tim2io_int_f  <= 1'b1;  // raise interrupt
          tim2io_busy_f <= 1'b0;  // when  not counting - not busy
        end else begin
          count         <= count + 1'b1;
          tim2io_busy_f <= 1'b1;  // when counting - busy
        end
      end
    end
  end

endmodule : timer
