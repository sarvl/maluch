module timer #(
    parameter int unsigned CLK_HZ    = 1_000_000,  //1MHz
    parameter int unsigned PERIOD_US = 1000        //1 ms = 1000 us
) (
    input logic clk,
    input logic _reset,

    // ack should be pulsed high for clk cycle when the interrupt is serviced
    input  logic ack,
    output logic irq
);

  // number of clock cycles in the period
  localparam int CYCLES_PER_US = CLK_HZ / 1_000_000;
  localparam int PERIOD_CYCLES = PERIOD_US * CYCLES_PER_US;

  logic [$clog2(PERIOD_CYCLES)-1:0] count;

  always_ff @(posedge clk) begin
    if (!_reset) begin
      count <= '0;
      irq   <= 1'b0;
    end else begin
      if (irq) begin
        if (ack) begin  // clear interrupt
          irq   <= 1'b0;
          count <= '0;
        end
      end else begin
        if (count == PERIOD_CYCLES - 1) begin
          irq <= 1'b1;  // raise interrupt
        end else begin
          count <= count + 1'b1;
        end
      end
    end
  end

endmodule : timer
