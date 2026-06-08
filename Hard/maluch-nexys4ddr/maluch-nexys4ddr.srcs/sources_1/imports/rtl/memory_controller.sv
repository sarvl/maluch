`timescale 1ns / 1ps
`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif

module memory_controller #(
    parameter int AddrSize = 16,
    parameter int DataSize = 16
) (
    input  logic                     clk_gpu,
    input  logic                     clk_cpu,
    input  logic                     _reset,
    output logic [31:0]              mem2core_instr,
    output logic [15:0]              mem2core_data_r,
    input  logic [15:0]              core2mem_instr_pointer,
    input  logic [15:0]              core2mem_addr,
    input  logic [15:0]              core2mem_data_w,
    input  logic                     core2mem_write_en,
    input  logic [15:0]              prog2mem_data,
    input  logic [15:0]              prog2mem_addr,
    input  logic                     prog2mem_w_en,
    input  logic                     prog2mem_stream_en, 
    output logic [15:0]              mem2gpu_data,
    input  logic [15:0]              gpu2mem_addr
);
    // Port A addresses and data outputs (Instructions)
    logic [13:0] ram1_addr_a, ram2_addr_a, rom1_addr_a, rom2_addr_a;
    logic [15:0] ram1_data_a, ram2_data_a, rom1_data_a, rom2_data_a;

    // Port B addresses, write data, read data outputs, and write enables (Data Read/Write)
    logic [13:0] ram_addr_b, rom_addr_b;
    logic [15:0] ram1_data_b, ram2_data_b, rom1_data_b, rom2_data_b;
    logic [15:0] ram_w_data, rom_w_data;
    logic        ram1_w_en, ram2_w_en, rom1_w_en, rom2_w_en;

    // Instantiations using the true dual-port sync_ram
    sync_ram #(.AddrSize(14), .DataSize(16)) RAM1 (
        .clk(clk_gpu),
        .pa_addr_i(ram1_addr_a), .pa_data_o(ram1_data_a),
        .pb_addr_i(ram_addr_b),  .pb_data_o(ram1_data_b),
        .w_data_i(ram_w_data),   .w_en(ram1_w_en)
    );
    
    sync_ram #(.AddrSize(14), .DataSize(16)) RAM2 (
        .clk(clk_gpu),
        .pa_addr_i(ram2_addr_a), .pa_data_o(ram2_data_a),
        .pb_addr_i(ram_addr_b),  .pb_data_o(ram2_data_b),
        .w_data_i(ram_w_data),   .w_en(ram2_w_en)
    );
    
    sync_ram #(.AddrSize(14), .DataSize(16), .INIT_FILE("rom1.hex")) ROM1 (
        .clk(clk_gpu),
        .pa_addr_i(rom1_addr_a), .pa_data_o(rom1_data_a),
        .pb_addr_i(rom_addr_b),  .pb_data_o(rom1_data_b),
        .w_data_i(rom_w_data),   .w_en(rom1_w_en)
    );
    
    sync_ram #(.AddrSize(14), .DataSize(16), .INIT_FILE("rom2.hex")) ROM2 (
        .clk(clk_gpu),
        .pa_addr_i(rom2_addr_a), .pa_data_o(rom2_data_a),
        .pb_addr_i(rom_addr_b),  .pb_data_o(rom2_data_b),
        .w_data_i(rom_w_data),   .w_en(rom2_w_en)
    );

    logic [14:0] vram_cpu_addr;
    sync_ram_dual #(.AddrSize(15), .DataSize(16)) VRAM (
        .clk(clk_gpu), .pa_addr_i(vram_cpu_addr), .pa_data_o(),
        .pb_addr_i(gpu2mem_addr[14:0]), .pb_data_o(mem2gpu_data),
        .w_addr_i(vram_cpu_addr), .w_data_i(core2mem_data_w), .w_en(core2mem_write_en && !core2mem_addr[15] && !clk_cpu)
    );

    always_comb begin
        logic [15:0] pc;
        logic [15:0] addr;

        pc = core2mem_instr_pointer;
        addr = core2mem_addr;

        // Defaults for Port A (Instruction Fetch)
        ram1_addr_a = 0; ram2_addr_a = 0; rom1_addr_a = 0; rom2_addr_a = 0;

        // Defaults for Port B (Data Read/Write)
        ram_addr_b = 0; rom_addr_b = 0; vram_cpu_addr = 0;
        ram1_w_en = 0; ram2_w_en = 0; rom1_w_en = 0; rom2_w_en = 0;
        ram_w_data = core2mem_data_w; rom_w_data = prog2mem_data;

        if (prog2mem_stream_en) begin
            // Programmer mode: Use Port B for all writes/reads
            ram_addr_b = prog2mem_addr[14:1];
            rom_addr_b = prog2mem_addr[14:1];
            if (prog2mem_w_en) begin
                if (prog2mem_addr[15]) begin
                    ram1_w_en = !prog2mem_addr[0]; ram2_w_en = prog2mem_addr[0];
                end else begin
                    rom1_w_en = !prog2mem_addr[0]; rom2_w_en = prog2mem_addr[0];
                end
            end
        end else begin
            // Normal execution mode:
            // 1. Port A is dedicated to instruction fetch from PC
            ram1_addr_a = pc[0] ? pc[14:1] + 1'b1 : pc[14:1];
            ram2_addr_a = pc[14:1];
            rom1_addr_a = ram1_addr_a;
            rom2_addr_a = ram2_addr_a;

            // 2. Port B is dedicated to data reads and writes from core2mem_addr
            ram_addr_b = addr[14:1];
            rom_addr_b = addr[14:1];
            vram_cpu_addr = addr[14:0];

            if (core2mem_write_en && addr[15] && !clk_cpu) begin
                ram1_w_en = !addr[0]; ram2_w_en = addr[0];
            end
        end
    end

    // Combinationally drive instruction from stable Port A outputs
    assign mem2core_instr = core2mem_instr_pointer[15] ?
        (core2mem_instr_pointer[0] ? {ram2_data_a, ram1_data_a} : {ram1_data_a, ram2_data_a}) :
        (core2mem_instr_pointer[0] ? {rom2_data_a, rom1_data_a} : {rom1_data_a, rom2_data_a});

    // Combinationally drive data reads from Port B outputs
    always_comb begin
        if (core2mem_addr[15])
            mem2core_data_r = core2mem_addr[0] ? ram2_data_b : ram1_data_b;
        else
            mem2core_data_r = core2mem_addr[0] ? rom2_data_b : rom1_data_b;
    end
endmodule