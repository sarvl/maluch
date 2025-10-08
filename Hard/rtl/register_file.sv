
module register_file #(
    parameter DataWidth = 16,
    parameter NumRegs = 16
)(
    input logic  clk,
    input logic  reg_w_en,
    input logic [15:0]  reg_in,
    input logic [3:0]  addr_in,
    input logic [3:0]  addr_out1,
    input logic [3:0]  addr_out2,

    output logic [15:0] reg_out1,
    output logic [15:0] reg_out2
);

    logic [DataWidth-1:0] regs[NumRegs];

    always_ff @(posedge clk) begin

        if (reg_w_en) begin
            regs[addr_in] <= reg_in;
        end
    end

    assign reg_out1 = regs[addr_out1];
    assign reg_out2 = regs[addr_out2];

endmodule
