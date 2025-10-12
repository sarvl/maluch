
// MAIN MODULE

module memory #(
    parameter int AddrSize=16,
    parameter int DataSize=16
)(
    input logic clk,
    input logic drv_ram,
    input logic write_en,
    input logic [AddrSize-1:0]  addr,
    input logic [DataSize-1:0]  data_in,
    output logic [2*DataSize-1:0]  data_out
);

    logic   ram1_en = !addr[0];
    logic   ram2_en = addr[0];
    logic   vram_en = addr[DataSize-1];

    logic [DataSize-1:0]    ram1_data_o;
    logic [DataSize-1:0]    ram2_data_o;
    logic [DataSize-1:0]    vram_data_o;

    logic   _w_ram1_en = write_en && ram1_en;
    logic   _w_ram2_en = write_en && ram2_en;
    logic   _w_vram_en = write_en && vram_en;

    ram #(.AddrSize(14)) RAM1   (.clk(clk), .data_in(data_in), .data_out(ram1_data_o), .addr(addr[14:1]), .write_en(_w_ram1_en));
    ram #(.AddrSize(14)) RAM2   (.clk(clk), .data_in(data_in), .data_out(ram2_data_o), .addr(addr[14:1]), .write_en(_w_ram2_en));
    ram #(.AddrSize(15)) VRAM   (.clk(clk), .data_in(data_in), .data_out(vram_data_o), .addr(addr[14:0]), .write_en(_w_vram_en));

    logic [DataSize-1:0]    ram_d_out;

    assign ram_d_out = ram1_en ? ram1_data_o : (ram2_en ? ram2_data_o : vram_data_o);

    // ----------------------------------------
    //  ROM Logic
    // ----------------------------------------

    logic [DataSize-1:0]    rom1_data;
    logic [DataSize-1:0]    rom2_data;

    logic [AddrSize-3:0]    rom1_addr;
    logic [AddrSize-3:0]    rom2_addr;

    assign rom1_addr = addr[0] ?  addr[14:1] + 1 : addr[14:1];
    assign rom2_addr = addr[14:1];

    rom #(.AddrSize(14)) ROM1    (.clk(clk), .data_out(rom1_data), .addr(rom1_addr));
    rom #(.AddrSize(14)) ROM2    (.clk(clk), .data_out(rom2_data), .addr(rom2_addr));

    logic [2*DataSize-1:0]  rom_d_out;

    assign rom_d_out = addr[0] ? {rom2_data, rom1_data} : {rom1_data, rom2_data};


    assign data_out = drv_ram ? {{16{1'b0}}, ram_d_out} : rom_d_out;

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
    output logic [DataSize-1:0]  data_out
);

    reg [DataSize-1:0] block [Depth];

    always_ff @(posedge clk) begin
        if (write_en) begin
            block[addr] <= data_in;
        end else begin
            data_out <= block[addr];
        end
    end

endmodule

module rom #(
    parameter int AddrSize=15,
    parameter int DataSize=16,
    parameter int Depth=$rtoi($pow(2,AddrSize))
)(
    input logic                 clk,
    input logic [AddrSize-1:0]  addr,
    output logic [DataSize-1:0]  data_out
);

    reg [DataSize-1:0] block [Depth];

    always_ff @(posedge clk) begin
        data_out <= block[addr];
    end

endmodule
