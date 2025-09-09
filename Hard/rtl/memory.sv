
// MAIN MODULE

module memory #(
    parameter int AddrSize=16,
    parameter int DataSize=16
)(
    input logic [AddrSize-1:0]  addr,
    input logic [DataSize-1:0]  data_in,
    output logic [DataSize-1:0]  data_out,
    input logic write_en,
    input logic clk
);
    logic   c1_en = !addr[DataSize-1] && !addr[0];
    logic   c2_en = !addr[DataSize-1] && addr[0];
    logic   c3_en = addr[DataSize-1];
    logic   c4_en = addr[DataSize-1];

    logic [DataSize-1:0]    data_out_1;
    logic [DataSize-1:0]    data_out_2;
    logic [DataSize-1:0]    data_out_3;
    logic [DataSize-1:0]    data_out_4;

    logic _write_en_1 = c1_en && write_en;
    logic _write_en_2 = c2_en && write_en;
    logic _write_en_4 = c4_en && write_en;

    ram #(.AddrSize(14)) RAM1   (.clk(clk), .data_in(data_in), .data_out(data_out_1), .addr(addr[14:1]), .write_en(_write_en_1));
    ram #(.AddrSize(14)) RAM2   (.clk(clk), .data_in(data_in), .data_out(data_out_2), .addr(addr[14:1]), .write_en(_write_en_2));
    rom #(.AddrSize(15)) ROM    (.clk(clk), .data_out(data_out_3), .addr(addr[14:0]));
    ram #(.AddrSize(15)) VRAM   (.clk(clk), .data_in(data_in), .data_out(data_out_4), .addr(addr[14:0]), .write_en(_write_en_4));


    assign data_out = c1_en ? data_out_1 : (c2_en ? data_out_2 : (c3_en && !write_en ? data_out_3 : data_out_4));

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
