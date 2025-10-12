`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif
`ifndef CORE
    `include "core_i.sv"
    `define CORE
`endif



module decoder (
    core_i.Decoder CoreBus
);
    import types::instr_t;
    instr_t i;
    assign i = CoreBus.instruction;

    // registers driver
    always_comb begin
        CoreBus.addr_out1 = i.dest_reg;
        CoreBus.addr_out2 = i.src_reg;

        CoreBus.addr_in = i.dest_reg;
        CoreBus.reg_w_en = i.opcode inside {4'b0001, 4'b0010} ? 1 : 0;
    end


    // ALU driver
    always_comb begin
        CoreBus.alu_ctrl = i.funct;
        CoreBus.src1 = CoreBus.reg_out1;
        CoreBus.src2 = i.imm_valid ? i.imm : CoreBus.reg_out2;
    end

    logic [15:0]    _output;

    // outcome driver
    always_comb begin
        case (i.opcode)
            default: _output = CoreBus.alu_ret;
        endcase
    end

    assign CoreBus.reg_in = _output;

endmodule
