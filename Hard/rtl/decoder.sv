`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif
`ifndef CORE
    `include "core_i.sv"
    `define CORE
`endif



module decoder (
    input logic [31:0]  instruction,
    input logic [15:0]  alu_ret,
    input logic [15:0]  reg_out1,
    input logic [15:0]  reg_out2,

    output logic [15:0] src1,
    output logic [15:0] src2,
    output logic [2:0] alu_ctrl,
    output logic [15:0] reg_in,
    output logic [3:0] addr_in,
    output logic [3:0] addr_out1,
    output logic [3:0] addr_out2,
    output logic reg_w_en
);
    import types::instr_t;
    instr_t i;
    assign i = instruction;

    // registers driver
    always_comb begin
        addr_out1 = i.dest_reg;
        addr_out2 = i.src_reg;

        addr_in = i.dest_reg;
        reg_w_en = i.opcode inside {4'b0001, 4'b0010} ? 1 : 0;
    end


    // ALU driver
    always_comb begin
        alu_ctrl = i.funct;
        src1 = reg_out1;
        src2 = i.imm_valid ? i.imm : reg_out2;
    end

    logic [15:0]    _output;

    // outcome driver
    always_comb begin
        case (i.opcode)
            default: _output = alu_ret;
        endcase
    end

    assign reg_in = _output;

endmodule
