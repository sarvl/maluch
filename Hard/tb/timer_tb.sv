`timescale 1us / 10ns
`include "timer.sv"

module timer_tb ();
  logic clk;
  logic _reset;
  logic ack;
  logic irq;

  timer timer (
      .clk(clk),
      ._reset(_reset),
      .ack(ack),
      .irq(irq)
  );

  always #0.5 clk = ~clk;  //1MHz clock

  initial begin
    clk    = 1'b0;
    _reset = 1'b1;
    ack    = 1'b0;

    // reset
    repeat (2) @(posedge clk);
    _reset = 1'b0;
    repeat (2) @(posedge clk);
    _reset = 1'b1;

    // wait for irq to go high
    wait (irq === 1'b1);
    repeat (500) @(posedge clk);

    // service (ack) for 1 cycle
    @(posedge clk) ack <= 1'b1;
    @(posedge clk) ack <= 1'b0;

    // next interrupt
    @(posedge clk) wait (irq === 1'b1);

    // service again
    @(posedge clk) ack <= 1'b1;
    @(posedge clk) ack <= 1'b0;
    repeat (5) @(posedge clk);
    $finish;
  end

  initial begin
    $dumpfile("waveforms/timer_tb.fst");
    $dumpvars(0, timer_tb);
  end
endmodule : timer_tb
