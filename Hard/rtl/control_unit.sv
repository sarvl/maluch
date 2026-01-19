`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif

module control_unit (
    input logic [31:0]  instruction,

    output logic        csr_flags_we,

    // ALU
    input logic [15:0]  alu_ret,
    output logic [15:0] src1,
    output logic [15:0] src2,
    output logic [2:0] alu_ctrl,

    // Registers
    input logic [15:0]  reg_out1,
    input logic [15:0]  reg_out2,
    output logic [15:0] reg_in,
    output logic [3:0] addr_in,
    output logic [3:0] addr_out1,
    output logic [3:0] addr_out2,
    output logic reg_w_en,

    // IO
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

    // Opcodes
    localparam OPCODE_ALU = 4'b0010;
    localparam OPCODE_LDW = 4'b1000;
    localparam OPCODE_STW = 4'b1001;
    localparam OPCODE_OUT = 4'b0111;
    localparam OPCODE_IN  = 4'b0110;

    // Registers driver
    always_comb begin
        addr_out1 = 4'b0000;
        addr_out2 = 4'b0000;
        addr_in = 4'b0000;
        reg_w_en = 0;
        reg_in = 16'h0000;

        if (i.opcode == OPCODE_ALU) begin
            addr_out1 = i.dest_reg;
            addr_out2 = i.src_reg;
            addr_in = i.dest_reg;
            reg_in = alu_ret;
            reg_w_en = 1;
        end else if (i.opcode == OPCODE_LDW) begin
            addr_out2 = i.src_reg;
            addr_in = i.dest_reg;
            reg_in = mem_ctrl_data_r;
            reg_w_en = 1;
        end else if (i.opcode == OPCODE_IN) begin
            addr_in = i.dest_reg;
            reg_in = io_data_r;
            reg_w_en = 1;
        end
    end

    // IO driver
    always_comb begin
        io_addr = 3'b000;
        io_w_en = 0;
        io_r_en = 0;
        io_data_w = 16'h0000;

        if (i.opcode == OPCODE_OUT) begin
            io_addr = i.funct;
            io_w_en = 1;
            io_data_w = reg_out2;
        end else if (i.opcode == OPCODE_IN) begin
            io_addr = i.funct;
            io_r_en = 1;
        end
    end

    // Memory controller driver
    always_comb begin
        mem_ctrl_addres = 16'h0000;
        mem_ctrl_data_w = 16'h0000;
        mem_ctrl_write_en = 0;

        if (i.opcode == OPCODE_STW) begin
            mem_ctrl_addres = reg_out2;
            mem_ctrl_data_w = reg_out1;
            mem_ctrl_write_en = 1;
        end else if (i.opcode == OPCODE_LDW) begin
            mem_ctrl_addres = reg_out2;
        end
    end

    // ALU driver
    always_comb begin
        alu_ctrl = 3'b000;
        src1 = 16'h0000;
        src2 = 16'h0000;
        csr_flags_we = 0;

        if (i.opcode == OPCODE_ALU) begin
            alu_ctrl = i.funct;
            src1 = reg_out1;
            src2 = i.imm_valid ? i.imm : reg_out2;
            csr_flags_we = 1;
        end
    end

endmodule
