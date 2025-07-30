`include "alu.sv"
`include "decoder.sv"
`include "types.sv"
`include "register_file.sv"

import Types::*;

module top (
    input logic clk,
    input logic _reset,
    input logic [31:0] instr_in
);

    logic [15:0] dest, src2, _reg2, _imm, reg_data;
    logic [3:0] dest_addr, src_addr;
    instr_t codes;
    csr_t csr;
    logic _reg_write;

    decoder IP (
        .clk, .instr_in, ._reset,
        .code_bus(codes), .imm(_imm), .dest_reg(dest_addr), .src_reg(src_addr)
    );

    register_file registers (
        .clk, .writeEn(_reg_write),
        .readAddr1(dest_addr), .readAddr2(src_addr),
        .writeAddr(dest_addr), .writeData(reg_data),
        .readData1(dest), .readData2(_reg2)
    );


    // Handler
    logic [15:0] _alu_out;

    alu ALU (
        .src1(dest),
        .src2,
        .aopcode(codes.funct),
        .csr,
        .result(_alu_out)
    );

    always_comb src2 = (codes.imm_present) ? _imm : _reg2;

    always_comb begin

        case (codes.opcode)
            4'b0001: begin // mov
                reg_data = src2;
                _reg_write = 1;
            end
            4'b0010: begin // alu
                reg_data = _alu_out;
                _reg_write = 1;
            end
            4'b0011: begin //cmp
                reg_data = _alu_out;
                _reg_write = 0;
            end
            4'b1000: begin // ldw
                reg_data = 'x;
                _reg_write = 1;
            end
            default: begin
                reg_data = 'x;
                _reg_write = 0;
            end
        endcase
    end


    initial begin
        $dumpfile("waveforms/core.fst");
        $dumpvars(0, top);
    end

endmodule
