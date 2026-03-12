module interrupt_controller (
    input logic         clk,
    input logic         _reset,
    input logic [7:0] masked_int_flags,
    input logic         iret_f,
    output logic        irq_f,
    output logic [3:0]  irq_relative_addr
);

    logic sig_trigg;

    assign irq_f = sig_trigg;

    always_ff @(posedge clk or posedge _reset) begin
        if (_reset) begin
            sig_trigg <= 0;
        end else begin
            if (sig_trigg)
                sig_trigg <= 0;
            else if (masked_int_flags != 0)
                sig_trigg <= 1;
        end
    end

    always_comb begin
        unique casez (masked_int_flags)
            8'b1??????? : irq_relative_addr = 'h0;
            8'b01?????? : irq_relative_addr = 'h2;
            8'b001????? : irq_relative_addr = 'h4;
            8'b0001???? : irq_relative_addr = 'h6;
            8'b00001??? : irq_relative_addr = 'h8;
            8'b000001?? : irq_relative_addr = 'hA;
            8'b0000001? : irq_relative_addr = 'hC;
            8'b00000001 : irq_relative_addr = 'hE;
            8'b00000000 : irq_relative_addr = 'hF;
        endcase
    end

endmodule