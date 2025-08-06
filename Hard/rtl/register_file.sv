`include "core.sv"

module register_file #(
    parameter DataWidth = 16,
    parameter NumRegs = 16
)(
    core.RegFile CoreBus
);

    logic [DataWidth-1:0] regs[NumRegs];

    always_ff @(posedge CoreBus.clk) begin

        if (CoreBus.reg_w_en) begin
            regs[CoreBus.addr_in] <= CoreBus.reg_in;
        end
    end

    assign CoreBus.reg_out1 = regs[CoreBus.addr_out1];
    assign CoreBus.reg_out2 = regs[CoreBus.addr_out2];

endmodule
