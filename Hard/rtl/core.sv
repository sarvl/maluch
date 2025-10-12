`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif
`ifndef CORE
    `include "core_i.sv"
    `define CORE
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

    core_i CoreBus(.clk, .reset(_reset));
    counter IP(.CoreBus(CoreBus.IP));
    decoder Decoder(.CoreBus(CoreBus.Decoder));
    alu ALU(.CoreBus(CoreBus.ALU));
    register_file register(.CoreBus(CoreBus.RegFile));

    assign pointer = CoreBus.instr_pointer;
    assign CoreBus.instruction = instr_in;

endmodule
