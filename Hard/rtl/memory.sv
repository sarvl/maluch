
// MAIN MODULE

module memory #(
    parameter int AddrSize=16,
    parameter int DataSize=16
)(
    input   logic                   clk,
    input   logic                   write_en,
    input   logic [AddrSize-1:0]    addr,
    input   logic [DataSize-1:0]    data_in, 
    input logic [AddrSize-1:0]      mem_vram_addr,
    input logic                     mem_prog_write_en,

    output  logic [DataSize-1:0]    mem_vram_data,
    output  logic [2*DataSize-1:0]  data_out


);
    // RAM: 0x8000 - 0xFFFF
    // VRAM: 0x0000 - 0x7FFF WRITE ONLY 
    // ROM: 0x0000 - 0x7FFF READ ONLY 




    // ----------------------------------------
    //  RAM + VRAM Logic
    // ----------------------------------------


    logic   ram_en = addr[AddrSize-1];
    logic   ram1_en = !addr[0] && ram_en;
    logic   ram2_en = addr[0] && ram_en;


    logic [DataSize-1:0]    ram1_data_o;
    logic [DataSize-1:0]    ram2_data_o;
    logic [DataSize-1:0]    ram_d_out;

    logic   _w_ram1_en = write_en && ram1_en;
    logic   _w_ram2_en = write_en && ram2_en;
    logic   _w_vram_en = write_en && !ram_en;

    ram #(.AddrSize(14)) RAM1   (.clk(clk), .data_in(data_in), .data_out(ram1_data_o), .addr(addr[14:1]), .write_en(_w_ram1_en));
    ram #(.AddrSize(14)) RAM2   (.clk(clk), .data_in(data_in), .data_out(ram2_data_o), .addr(addr[14:1]), .write_en(_w_ram2_en));
    ram #(.AddrSize(15)) VRAM   (.clk(clk), .data_in(data_in), .data_out(mem_vram_data), .addr(mem_vram_addr[14:0]), .write_en(_w_vram_en));



   


    // ----------------------------------------
    //  ROM Logic
    // ----------------------------------------

    logic [DataSize-1:0]    rom1_data_o;
    logic [DataSize-1:0]    rom2_data_o;

    logic [AddrSize-3:0]    rom1_addr;
    logic [AddrSize-3:0]    rom2_addr;

    logic rom_write_en;

    assign rom_write_en = mem_prog_write_en;
    assign rom1_addr = addr[0] ?  addr[14:1] + 1 : addr[14:1];
    assign rom2_addr = addr[14:1];

    rom #(.AddrSize(14)) ROM1    (.clk(clk), .data_out(rom1_data_o), .addr(rom1_addr),data_in(data_in), write_en(rom_write_en));
    rom #(.AddrSize(14)) ROM2    (.clk(clk), .data_out(rom2_data_o), .addr(rom2_addr), data_in(data_in), write_en(rom_write_en));

    logic [2*DataSize-1:0]  rom_d_out;


    // ----------------------------------------
    //  outputs Logic
    // ----------------------------------------


    assign ram_d_out = ram1_en ? ram1_data_o : (ram2_en ? ram2_data_o : vram_data_o);

    assign rom_d_out = addr[0] ? {rom2_data, rom1_data} : {rom1_data, rom2_data};

    assign data_out = addr[AddrSize-1] ? {{16{1'b0}}, ram_data_o} : rom_d_out;

endmodule

module ram #(
    parameter int AddrSize=15,
    parameter int DataSize=16,
    parameter int Depth=$rtoi($pow(2,AddrSize))
)(
    input logic                 clk,
    input logic                 write_en,
    input logic [AddrSize-1:0]  addr,
    input logic [DataSize-1:0]  data_in,
    output logic [DataSize-1:0] data_out
);

    reg [DataSize-1:0] block [Depth];

    always_ff @(negedge clk) begin
        if (write_en) begin
            block[addr] <= data_in;
        end 
    end

    always_comb begin
        data_out = block[addr];
    end
endmodule

module rom #(
    parameter int AddrSize=15,
    parameter int DataSize=16,
    parameter int Depth=$rtoi($pow(2,AddrSize))
)(
    input logic                 clk,
    input logic [AddrSize-1:0]  addr,
    input logic [DataSize-1:0]  data_in,
    input logic                 write_en,
    output logic [DataSize-1:0]  data_out
);

    reg [DataSize-1:0] block [Depth];

    always_ff @(negedge clk) begin
        if (write_en) begin
            block[addr] <= data_in;
        end 
    end

    always_comb begin
        data_out = block[addr];
    end

endmodule
