`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif



module decoder (
    input logic [31:0]  instruction,

    //ALU
    input logic [15:0]  alu_ret,

    output logic [15:0] src1,
    output logic [15:0] src2,
    output logic [2:0] alu_ctrl,

    //Registers
    input logic [15:0]  reg_out1,
    input logic [15:0]  reg_out2,

    output logic [15:0] reg_in,
    output logic [3:0] addr_in,
    output logic [3:0] addr_out1,
    output logic [3:0] addr_out2,
    output logic reg_w_en,

    //IO output
    input logic [15:0] io_data_r,

    output logic [2:0] io_addr,
    output logic io_w_en,
    output logic io_r_en,
    output logic [15:0] io_data_w,

    // Memory controller
    input logic [15:0] mem_ctrl_data_r,

    output logic [15:0] mem_ctrl_addres,
    output logic [15:0] mem_ctrl_data_w,
    output logic mem_ctrl_write_en
);
    import types::instr_t;
    instr_t i;
    assign i = instruction;

    // registers driver
    always_comb begin
        addr_out1 = i.dest_reg;
        addr_out2 = i.src_reg;

        addr_in = i.dest_reg;
        reg_w_en = i.opcode inside {4'b0001, 4'b0010, 4'b0110, 4'b1000} ? 1 : 0;
        
        case (i.opcode)
            default: reg_in = alu_ret;
            4'b0110: reg_in = io_data_r;
            4'b1000: reg_in = mem_ctrl_data_r;
        endcase
    end

    // io driver
    always_comb begin
        io_addr = i.funct;
        io_w_en = (i.opcode == 4'b0111) ? 1 : 0;
        io_r_en = (i.opcode == 4'b0110) ? 1 : 0;
        io_data_w = i.imm_valid ? i.imm : reg_out2;
    end

    // memory controller driver
    always_comb begin
        mem_ctrl_addres = reg_out2;
        mem_ctrl_data_w = reg_out1;
        mem_ctrl_write_en = (i.opcode == 4'b1001) ? 1 : 0;
    end

    // ALU driver
    always_comb begin
        alu_ctrl = i.funct;
        src1 = reg_out1;
        src2 = i.imm_valid ? i.imm : reg_out2;
    end

endmodule
