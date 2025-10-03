`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif
`include "alu.sv"
`include "decoder.sv"
`include "register_file.sv"
`include "counter.sv"

/* verilator lint_off MULTITOP */
module core(
    input logic clk,
    input logic _reset,
    input logic [31:0] instr_in,

    output logic [31:0] pointer
);

    logic [31:0]    instruction;
    logic [31:0]    instr_pointer;
    logic [31:0]    _next_pointer;

    logic [15:0]    src1;
    logic [15:0]    src2;
    logic [15:0]    alu_ret;
    logic [2:0]     alu_ctrl;

    logic [15:0]    reg_out1;
    logic [15:0]    reg_out2;
    logic [15:0]    reg_in;
    logic [3:0]     addr_in;
    logic [3:0]     addr_out1;
    logic [3:0]     addr_out2;
    logic           reg_w_en;

    csr_t csr;
    csr_t _csr_next;

    // Driving outputs
    always_ff @(posedge clk) instr_pointer <= reset ? 1 : _next_pointer;

    // Driving csr
    always_ff @(posedge clk) csr <= _csr_next;

    counter IP(
        .instr_pointer,
        .csr,
        .instruction,
        .src2,
        ._next_pointer
    );
    decoder Decoder(
        .instruction,
        .alu_ret,
        .reg_out1,
        .reg_out2,

        .src1,
        .src2,
        .alu_ctrl,
        .reg_in,
        .addr_in,
        .addr_out1,
        .addr_out2,
        .reg_w_en
    );
    alu ALU(
        .alu_ctrl,
        .src1,
        .src2,

        .alu_ret,
        ._csr_next
    );
    register_file register(
        .clk,
        .reg_w_en,
        .reg_in,
        .addr_in,
        .addr_out1,
        .addr_out2,

        .reg_out1,
        .reg_out2
    );

    assign pointer = instr_pointer;
    assign instruction = instr_in;

endmodule
