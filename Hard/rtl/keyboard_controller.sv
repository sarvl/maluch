

module keyboard_controller(
input logic clk,

inout logic kclk,
input logic kdata,
input logic [2:0] io_addr;

output logic[7:0] data_out,
output logic busy_flag,
output logic int_flag
);

assign int_flag = data_rdy;
assign busy_flag = ~data_rdy;
assign data_out = (io_addr == 3'b001) ? data_buffer : 8'hz;
assign buffer_clr = (io_addr == 3'b001) & ~busy_flag;
assign kclk = kclk_en ? 1'bz : 1'b0;

logic kclkf, kdataf;
logic [7:0]datacur;
logic [7:0]dataprev;
logic [3:0]cnt;
logic cflag;

logic [15:0] scancode_pair;
logic sc_flag;
logic pflag;

logic shifting, shifting_nxt;
logic kclk_en;
logic set_sig, rst_sig;
logic write_en;

logic [7:0] ascii;
logic data_rdy;
logic buffer_clr;
logic[7:0] data_buffer;
    
debouncer #(
    .COUNT_MAX(19),
    .COUNT_WIDTH(5)
) db_clk(
    .clk(clk),
    .I(kclk),
    .O(kclkf)
);
debouncer #(
   .COUNT_MAX(19),
   .COUNT_WIDTH(5)
) db_data(
    .clk(clk),
    .I(kdata),
    .O(kdataf)
);

// Receiving scancodes
always@(negedge(kclkf))begin
    case(cnt)
        0:;//Start bit
        1:datacur[0]<=kdataf;
        2:datacur[1]<=kdataf;
        3:datacur[2]<=kdataf;
        4:datacur[3]<=kdataf;
        5:datacur[4]<=kdataf;
        6:datacur[5]<=kdataf;
        7:datacur[6]<=kdataf;
        8:datacur[7]<=kdataf;
        9:cflag<=1'b1;
        10:cflag<=1'b0;
    endcase

    if(cnt<=9)
        cnt<=cnt+1;
    else if(cnt==10)
        cnt<=0;
end

// Storing scancodes
always_ff @(posedge clk) begin
    if (cflag == 1'b1 && pflag == 1'b0) begin
        scancode_pair <= {dataprev, datacur};
        sc_flag <= 1'b1;
        dataprev <= datacur;
    end else
        sc_flag <= 'b0;

    pflag <= cflag;
end

// Managing ps2 clk
always_ff @(posedge clk) begin
    if(set_sig)
        kclk_en <= 1;
    else if(rst_sig)
        kclk_en <= 0;
end

// Handling shift key
always_ff @(posedge clk) begin
    shifting = shifting_nxt;
end

always_comb begin
    shifting_nxt = shifting;
    write_en_nxt = 0;
    set_sig = 0;

    if(sc_flag)
        unique case(scancode_pair)
            16'h0F12: shifting_nxt = 0;
            16'hxx12: shifting_nxt = 1;
            16'h0Fxx: write_en_nxt = 1;
            default:  set_sig = 1;
        endcase
end

//Translating scancodes to ASCII
always_comb begin
    unique case (scancode_pair[7:0])
        8'h1C: ascii = shifting ? "A" : "a"; // A
        8'h32: ascii = shifting ? "B" : "b"; // B
        8'h21: ascii = shifting ? "C" : "c"; // C
        8'h23: ascii = shifting ? "D" : "d"; // D
        8'h24: ascii = shifting ? "E" : "e"; // E
        8'h2B: ascii = shifting ? "F" : "f"; // F
        8'h34: ascii = shifting ? "G" : "g"; // G
        8'h33: ascii = shifting ? "H" : "h"; // H
        8'h43: ascii = shifting ? "I" : "i"; // I
        8'h3B: ascii = shifting ? "J" : "j"; // J
        8'h42: ascii = shifting ? "K" : "k"; // K
        8'h4B: ascii = shifting ? "L" : "l"; // L
        8'h3A: ascii = shifting ? "M" : "m"; // M
        8'h31: ascii = shifting ? "N" : "n"; // N
        8'h44: ascii = shifting ? "O" : "o"; // O
        8'h4D: ascii = shifting ? "P" : "p"; // P
        8'h15: ascii = shifting ? "Q" : "q"; // Q
        8'h2D: ascii = shifting ? "R" : "r"; // R
        8'h1B: ascii = shifting ? "S" : "s"; // S
        8'h2C: ascii = shifting ? "T" : "t"; // T
        8'h3C: ascii = shifting ? "U" : "u"; // U
        8'h2A: ascii = shifting ? "V" : "v"; // V
        8'h1D: ascii = shifting ? "W" : "w"; // W
        8'h22: ascii = shifting ? "X" : "x"; // X
        8'h35: ascii = shifting ? "Y" : "y"; // Y
        8'h1A: ascii = shifting ? "Z" : "z"; // Z

        8'h16: ascii = shifting ? "!" : "1";
        8'h1E: ascii = shifting ? "@" : "2";
        8'h26: ascii = shifting ? "#" : "3";
        8'h25: ascii = shifting ? "$" : "4";
        8'h2E: ascii = shifting ? "%" : "5";
        8'h36: ascii = shifting ? "^" : "6";
        8'h3D: ascii = shifting ? "&" : "7";
        8'h3E: ascii = shifting ? "*" : "8";
        8'h46: ascii = shifting ? "(" : "9";
        8'h45: ascii = shifting ? ")" : "0";
        8'h4E: ascii = shifting ? "_" : "-";
        8'h55: ascii = shifting ? "+" : "=";

        8'h5D: ascii = shifting ? "|" : "\\"; // Backslash
        8'h54: ascii = shifting ? "{" : "[";  // [
        8'h5B: ascii = shifting ? "}" : "]";  // ]
        8'h4C: ascii = shifting ? ":" : ";";  // ;
        8'h52: ascii = shifting ? "\"" : "'"; // '
        8'h41: ascii = shifting ? "<" : ",";  // ,
        8'h49: ascii = shifting ? ">" : ".";  // .
        8'h4A: ascii = shifting ? "?" : "/";  // /
        8'h0E: ascii = shifting ? "~" : "`";  // `
        8'h29: ascii = " ";                       // Space
        8'h5A: ascii = 8'h0A;                     // Enter
        8'h66: ascii = 8'h08;                     // Backspace
        8'h0D: ascii = 8'h09;                     // Tab
        8'h76: ascii = 8'h1B;                     // ESC

        default: ascii = 8'h00;
    endcase
end


//Output buffer
always_ff @(posedge clk) begin
    rst_sig <= 0;
    if(write_en) begin
        data_buffer <= ascii;
        data_rdy <= 1;
    end else if (buffer_clr) begin
        data_rdy <= 0;
        rst_sig <= 1;
    end
end

endmodule