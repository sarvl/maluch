
// MAIN MODULE

module memory #(
    parameter int AddrSize=16,
    parameter int DataSize=16
)(
    input   logic                   clk,
    input   logic                   write_en,
    input   logic [AddrSize-1:0]    addr,                   
    input   logic [AddrSize-1:0]    instr_addr,             
    input   logic [DataSize-1:0]    data_in, 
    input logic [AddrSize-1:0]      mem_vram_addr,
    input logic                     mem_prog_write_en,

    output  logic [DataSize-1:0]    mem_vram_data,
    output  logic [DataSize-1:0]  data_out,               
    output  logic [2*DataSize-1:0]  instr_data_out         


);
    // RAM: 0x8000 - 0xFFFF
    // VRAM: 0x0000 - 0x7FFF WRITE ONLY 
    // ROM: 0x0000 - 0x7FFF READ ONLY 


    ram #(.AddrSize(14)) RAM1   (.clk(clk), .prog_data(data_in), .data_out(ram1_data_o), .data_addr(addr[14:1]), .data_write_en(_w_ram1_en),
    .prog_write_en(prog_ram1_en), .instr_addr(ram1_instr_addr), .instr_data_out(ram1_instr_data_o) );

    ram #(.AddrSize(14)) RAM2   (.clk(clk), .prog_data(data_in), .data_out(ram2_data_o), .data_addr(addr[14:1]), .data_write_en(_w_ram2_en),
    .prog_write_en(prog_ram2_en), .instr_addr(ram2_instr_addr), .instr_data_out(ram2_instr_data_o) );

    ram #(.AddrSize(15)) VRAM   (.clk(clk), .prog_data(data_in), .data_out(), .data_addr(addr[14:0]), 
    .data_write_en(_w_vram_en),.prog_write_en(1'b0),.instr_addr(mem_vram_addr[14:0]),.instr_data_out(mem_vram_data));


    rom #(.AddrSize(14)) ROM1    (.clk(clk), .instr_addr(rom1_instr_addr), .instr_data_out(rom1_instr_data_o),
    .data_out(rom1_data_o), .data_addr(rom1_addr), .prog_data(data_in), .write_en(rom1_write_en));

    rom #(.AddrSize(14)) ROM2    (.clk(clk), .instr_addr(rom2_instr_addr), .instr_data_out(rom2_instr_data_o),
    .data_out(rom2_data_o), .data_addr(rom2_addr), .prog_data(data_in), .write_en(rom2_write_en));

   



    // ----------------------------------------
    //  Instruction  Logic 
    // ----------------------------------------

    logic [DataSize-1:0]    ram1_instr_data_o;
    logic [DataSize-1:0]    ram2_instr_data_o;
    logic [2*DataSize-1:0]  ram_instr_data_out;
    logic [AddrSize-3:0]    ram1_instr_addr;
    logic [AddrSize-3:0]    ram2_instr_addr;   


    logic [2*DataSize-1:0]  rom_instr_data_out;
    logic [DataSize-1:0]    rom1_instr_data_o;
    logic [DataSize-1:0]    rom2_instr_data_o;
    logic [AddrSize-3:0]    rom1_instr_addr;
    logic [AddrSize-3:0]    rom2_instr_addr;

    assign ram1_instr_addr = instr_addr[0] ?  instr_addr[14:1] + 1 : instr_addr[14:1];
    assign ram2_instr_addr = instr_addr[14:1];

    assign rom1_instr_addr = instr_addr[0] ?  instr_addr[14:1] + 1 : instr_addr[14:1];
    assign rom2_instr_addr = instr_addr[14:1];

    assign ram_instr_data_out = instr_addr[0] ? {ram2_instr_data_o, ram1_instr_data_o} :
                                            {ram1_instr_data_o, ram2_instr_data_o};

    assign rom_instr_data_out = instr_addr[0] ? {rom2_instr_data_o, rom1_instr_data_o} :
                                            {rom1_instr_data_o, rom2_instr_data_o};

    assign instr_data_out = instr_addr[AddrSize-1] ? ram_instr_data_out : rom_instr_data_out;

    // ----------------------------------------
    //  Data Logic
    // ----------------------------------------
    logic [DataSize-1:0]    rom1_data_o;
    logic [DataSize-1:0]    rom2_data_o;
    logic [AddrSize-3:0]    rom1_addr;
    logic [AddrSize-3:0]    rom2_addr;
    logic [DataSize-1:0]  rom_d_out;
    logic rom1_write_en;
    logic rom2_write_en;

    logic   ram_en, ram1_en, ram2_en;
    logic [DataSize-1:0]    ram1_data_o, ram2_data_o, ram_d_out;
    logic _w_ram1_en, _w_ram2_en, _w_vram_en;
    logic prog_ram1_en, prog_ram2_en;
    assign ram_en  = addr[AddrSize-1];
    assign ram1_en = !addr[0] && ram_en;
    assign ram2_en =  addr[0] && ram_en;

    assign   _w_ram1_en = write_en && ram1_en;
    assign   _w_ram2_en = write_en && ram2_en;
    assign   _w_vram_en = write_en && !ram_en;
    assign   prog_ram1_en = mem_prog_write_en && ram1_en;
    assign   prog_ram2_en = mem_prog_write_en && ram2_en;

    assign rom1_write_en = (mem_prog_write_en && !instr_addr[0]);
    assign rom2_write_en = (mem_prog_write_en && instr_addr[0]);
    assign rom1_addr = addr[0] ?  addr[14:1] + 1 : addr[14:1];
    assign rom2_addr = addr[14:1];

    assign ram_d_out = ram1_en ? ram1_data_o : (ram2_en ? ram2_data_o : {16{1'b1}});

    assign rom_d_out = addr[0] ? rom2_data_o : rom1_data_o;

    assign data_out = ram_en ? ram_d_out : rom_d_out;




endmodule

module ram #(
    parameter int AddrSize=15,
    parameter int DataSize=16
)(
    input logic clk,

    input logic [AddrSize-1:0] instr_addr,
    output logic [DataSize-1:0] instr_data_out,

    input logic [AddrSize-1:0] data_addr,
    output logic [DataSize-1:0] data_out,

    input logic data_write_en,
    input logic prog_write_en,
    input logic [DataSize-1:0] prog_data
);

    logic [DataSize-1:0] block [2**AddrSize];

    always_ff @(negedge clk) begin
        if (prog_write_en)
            block[instr_addr] <= prog_data;
        else if  (data_write_en)
            block[data_addr] <= prog_data;
    end
    
    always_comb begin 
        instr_data_out = block[instr_addr];
        data_out = block[data_addr];
    end
endmodule

module rom #(
    parameter int AddrSize=15,
    parameter int DataSize=16
)(
    input logic clk,

    input logic [AddrSize-1:0] instr_addr,
    output logic [DataSize-1:0] instr_data_out,

    input logic [AddrSize-1:0] data_addr,
    output logic [DataSize-1:0] data_out,

    input logic write_en,
    input logic [DataSize-1:0] prog_data
);

    logic [DataSize-1:0] block [2**AddrSize];

    always_ff @(negedge clk) begin
        if (write_en)
            block[instr_addr] <= prog_data;

    end
    always_comb begin 
        instr_data_out = block[instr_addr];
        data_out = block[data_addr];
    end
endmodule
