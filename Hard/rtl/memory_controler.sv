`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif
`include "memory.sv"

import types::cpu_mem_bus_t;
module memory_controler(
    input logic         clk,
    input cpu_mem_bus_t cpu_bus,
    output logic [15:0] cpu_out,
    output logic [15:0] instr_out
);

    logic ptr = 0;
    always_ff@(posedge clk) if (cpu_bus.ptr_valid) ptr <= ptr + 1;

    logic [15:0] ptr_addr;
    always_ff@(posedge clk) begin
        ptr_addr <= ptr ? cpu_bus.ptr[31:16] : cpu_bus.ptr[15:0];
    end


    logic [15:0] addr;
    assign addr =   cpu_bus.ptr_valid ? ptr_addr :
                    cpu_bus.addr_valid ? cpu_bus.addr : '0;

    logic [15:0] data_in;
    assign data_in = cpu_bus.data2mem;

    logic [15:0] data_out;
    always_ff@(posedge clk) begin
        if (cpu_bus.ptr_valid) instr_out <= data_out;
        if (cpu_bus.addr_valid) cpu_out <= data_out;
    end


    memory Memory(.clk(clk), .write_en(cpu_bus.write), .addr(addr), .data_in(data_in), .data_out(data_out));

endmodule
