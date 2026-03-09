module memory_controller #(
    parameter int AddrSize = 16,
    parameter int DataSize = 16
) (
    input  logic                     clk,

    // Memory ports
    output logic                     mem_write_en,
    output logic [AddrSize-1:0]      mem_addr,
    output logic [7:0]               mem_vram_addr,
    output logic [DataSize-1:0]      mem_data_in,
    output logic                     mem_prog_write_en,
    input  logic [DataSize-1:0]      mem_vram_data,
    input  logic [2*DataSize-1:0]    mem_data_out,
    input  logic [2*DataSize-1:0]    mem_instr_data_out,

    // Core
    output logic [31:0]              mem2core_instr,
    output logic [15:0]              mem2core_data_r,
    input  logic [15:0]              core2mem_instr_pointer,
    input  logic [15:0]              core2mem_addr,
    input  logic [15:0]              core2mem_data_w,
    input  logic                     core2mem_write_en,

    // Programmer
    input  logic [15:0]              prog2mem_data,
    input  logic [15:0]              prog2mem_addr,
    input  logic                     prog2mem_w_en,     // sygnał że dane są poprawne i można zapisać
    input  logic                     prog2mem_stream_en, // programator programuje rom, reszta na resecie i wyjebane

    // GPU
    output logic [15:0]              mem2gpu_data,
    input  logic [7:0]               gpu2mem_addr
);

assign mem_prog_write_en = prog2mem_w_en;
assign mem_vram_addr     = gpu2mem_addr;
assign mem2gpu_data      = mem_vram_data; 

always_comb begin

    
    mem2core_data_r = 16'b0;
    mem_data_in     = 16'b0;
    mem_addr        = 16'b0;
    mem_write_en    = 1'b0;
    mem2core_instr  = mem_instr_data_out; 
    if (prog2mem_stream_en) begin
        if(prog2mem_w_en) begin 
            mem_addr     = prog2mem_addr;
            mem_data_in  = prog2mem_data;
        end
    end
    else begin
      
        if (core2mem_write_en) begin
            mem_write_en = 1'b1;
            mem_data_in  = core2mem_data_w;
        end else begin
            mem2core_data_r = mem_data_out[15:0];
        end
    end
end

always_ff @(negedge clk) begin
    if (prog2mem_w_en) begin
        mem_addr <= prog2mem_addr;
    end else begin
        mem_addr <= core2mem_addr; 
    end
end

always_ff @(posedge clk) begin
    mem_addr <= core2mem_instr_pointer;
end
endmodule