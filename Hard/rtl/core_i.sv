`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif



/* verilator lint_off MODDUP */
interface core_i(
    input logic clk,
    input logic reset
);

import types::csr_t;

logic [31:0]    instruction;
logic [31:0]    instr_pointer;
logic [31:0]    _next_pointer;


modport IP(
    input   instr_pointer,
    input   csr,
    input   instruction,
    input   src2,
    output  _next_pointer
);

// Driving outputs
always_ff @(posedge clk) instr_pointer <= reset ? 1 : _next_pointer;

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

modport Decoder(
    input   instruction,
    input   alu_ret,
    input   reg_out1,
    input   reg_out2,

    output  src1,
    output  src2,
    output  alu_ctrl,
    output  reg_in,
    output  addr_in,
    output  addr_out1,
    output  addr_out2,
    output  reg_w_en
);

csr_t csr;
csr_t _csr_next;
always_ff @(posedge clk) csr <= _csr_next;

modport ALU(
    input   alu_ctrl,
    input   src1,
    input   src2,

    output  alu_ret,
    output  _csr_next
);


modport RegFile(
    input   clk,
    input   reg_w_en,
    input   reg_in,
    input   addr_in,
    input   addr_out1,
    input   addr_out2,

    output  reg_out1,
    output  reg_out2
);



endinterface
