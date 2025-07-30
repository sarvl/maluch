module register_file #(
    parameter DataWidth = 16,
    parameter NumRegs = 16,
    parameter IndexWidth = $clog2(NumRegs)
)(
    input logic clk,
    input logic writeEn,
    input logic [IndexWidth-1:0] writeAddr,
    input logic [ DataWidth-1:0] writeData,
    input logic [IndexWidth-1:0] readAddr1,
    input logic [IndexWidth-1:0] readAddr2,
    output logic [ DataWidth-1:0] readData1,
    output logic [ DataWidth-1:0] readData2
);

    logic [DataWidth-1:0] regs[NumRegs];

    always_ff @(posedge clk) begin

        if (writeEn) begin
            regs[writeAddr] <= writeData;
        end

    end

    assign readData1 = regs[readAddr1];
    assign readData2 = regs[readAddr2];

endmodule
