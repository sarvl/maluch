`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif

module control_unit (
    input logic [31:0]  instruction,
    input logic [15:0]  instr_pointer_seq,

    output logic        csr_flags_we,
    output logic [15:0]  instr_pointer_ctrl,

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
    output logic sp_w_en,
    output logic [15:0] sp_in,

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

    // Constants
    localparam REG_SP = 4'b0010;

    // Opcodes
    localparam OPCODE_BRANCHING = 4'b010?;
    localparam OPCODE_MOV = 4'b0001;
    localparam OPCODE_ALU = 4'b0010;
    localparam OPCODE_CMP_TEST = 4'b0011;
    localparam OPCODE_LDW = 4'b1000;
    localparam OPCODE_STW = 4'b1001;
    localparam OPCODE_OUT = 4'b0111;
    localparam OPCODE_IN  = 4'b0110;
    localparam OPCODE_PUSH = 4'b1100;
    localparam OPCODE_PULL = 4'b1101;
    localparam OPCODE_CALL = 4'b1010;
    localparam OPCODE_RET  = 4'b1011;

    // Program counter driver
    always_comb begin
        instr_pointer_ctrl = 16'h0000;

        case (i.opcode) inside
            OPCODE_BRANCHING: begin
                instr_pointer_ctrl = i.imm_valid ? i.imm : reg_out2;
            end
            OPCODE_CALL: begin
                instr_pointer_ctrl = i.imm_valid ? i.imm : reg_out2;
            end
            OPCODE_RET: begin
                instr_pointer_ctrl = mem_ctrl_data_r;
            end
            default: ;
        endcase
    end

    // Registers driver
    always_comb begin
        addr_out1 = 4'b0000;
        addr_out2 = 4'b0000;
        addr_in = 4'b0000;
        reg_w_en = 0;
        reg_in = 16'h0000;
        sp_w_en = 0;
        sp_in = 16'h0000;

        case (i.opcode) inside
            OPCODE_BRANCHING: begin
                addr_out2 = i.src_reg;
            end
            OPCODE_MOV: begin
                addr_out2 = i.src_reg;
                addr_in = i.dest_reg;
                reg_in = i.imm_valid ? i.imm : reg_out2;
                reg_w_en = 1;
            end
            OPCODE_ALU: begin
                addr_out1 = i.dest_reg;
                addr_out2 = i.src_reg;
                addr_in = i.dest_reg;
                reg_in = alu_ret;
                reg_w_en = 1;
            end
            OPCODE_CMP_TEST: begin
                if (i.funct inside {3'b001, 3'b010}) begin
                    addr_out1 = i.dest_reg;
                    addr_out2 = i.src_reg;
                end
            end
            OPCODE_STW: begin
                addr_out1 = i.dest_reg;
                addr_out2 = i.src_reg;
            end
            OPCODE_LDW: begin
                addr_out2 = i.src_reg;
                addr_in = i.dest_reg;
                reg_in = mem_ctrl_data_r;
                reg_w_en = 1;
            end
            OPCODE_OUT: begin
                addr_out2 = i.src_reg;
            end
            OPCODE_IN: begin
                addr_in = i.dest_reg;
                reg_in = io_data_r;
                reg_w_en = 1;
            end
            OPCODE_PUSH: begin
                addr_out1 = REG_SP;
                addr_out2 = i.src_reg;
                sp_w_en = 1;
                sp_in = alu_ret;
            end
            OPCODE_PULL: begin
                addr_out1 = REG_SP;
                addr_in = i.dest_reg;
                reg_in = mem_ctrl_data_r;
                reg_w_en = 1;
                sp_w_en = 1;
                sp_in = alu_ret;
            end
            OPCODE_CALL: begin
                addr_out1 = REG_SP;
                addr_out2 = i.src_reg;
                sp_w_en = 1;
                sp_in = alu_ret;
            end
            OPCODE_RET: begin
                addr_out1 = REG_SP;
                sp_w_en = 1;
                sp_in = alu_ret;
            end
            default: ;
        endcase
    end

    // IO driver
    always_comb begin
        io_addr = 3'b000;
        io_w_en = 0;
        io_r_en = 0;
        io_data_w = 16'h0000;

        case (i.opcode) inside
            OPCODE_OUT: begin
                io_addr = i.funct;
                io_w_en = 1;
                io_data_w = i.imm_valid ? i.imm : reg_out2;
            end
            OPCODE_IN: begin
                io_addr = i.funct;
                io_r_en = 1;
            end
            default: ;
        endcase
    end

    // Memory controller driver
    always_comb begin
        mem_ctrl_addres = 16'h0000;
        mem_ctrl_data_w = 16'h0000;
        mem_ctrl_write_en = 0;

        case (i.opcode) inside
            OPCODE_STW: begin
                mem_ctrl_addres = i.imm_valid ? i.imm : reg_out2;
                mem_ctrl_data_w = reg_out1;
                mem_ctrl_write_en = 1;
            end
            OPCODE_LDW: begin
                mem_ctrl_addres = i.imm_valid ? i.imm : reg_out2;
            end
            OPCODE_PUSH: begin
                mem_ctrl_addres = alu_ret;
                mem_ctrl_data_w = i.imm_valid ? i.imm : reg_out2;
                mem_ctrl_write_en = 1;
            end
            OPCODE_PULL: begin
                mem_ctrl_addres = reg_out1;
            end
            OPCODE_CALL: begin
                mem_ctrl_addres = alu_ret;
                mem_ctrl_data_w = instr_pointer_seq;
                mem_ctrl_write_en = 1;
            end
            OPCODE_RET: begin
                mem_ctrl_addres = reg_out1;
            end
            default: ;
        endcase
    end

    // ALU driver
    always_comb begin
        alu_ctrl = 3'b000;
        src1 = 16'h0000;
        src2 = 16'h0000;
        csr_flags_we = 0;

        case (i.opcode) inside
            OPCODE_ALU: begin
                alu_ctrl = i.funct;
                src1 = reg_out1;
                src2 = i.imm_valid ? i.imm : reg_out2;
                csr_flags_we = 1;
            end
            OPCODE_CMP_TEST: begin
                if (i.funct inside {3'b001, 3'b010}) begin
                    alu_ctrl = i.funct;
                    src1 = reg_out1;
                    src2 = i.imm_valid ? i.imm : reg_out2;
                    csr_flags_we = 1;
                end
            end
            OPCODE_PUSH: begin
                src1 = reg_out1;
                src2 = 16'h0001;
                alu_ctrl = 3'b001;
            end
            OPCODE_PULL: begin
                src1 = reg_out1;
                src2 = 16'h0001;
                alu_ctrl = 3'b000;
            end
            OPCODE_CALL: begin
                src1 = reg_out1;
                src2 = 16'h0001;
                alu_ctrl = 3'b001;
            end
            OPCODE_RET: begin
                src1 = reg_out1;
                src2 = 16'h0001;
                alu_ctrl = 3'b000;
            end
            default: ;
        endcase
    end

endmodule
