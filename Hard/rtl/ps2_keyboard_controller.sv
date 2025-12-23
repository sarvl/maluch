

module ps2_keyboard_controller(
input logic clk,
input logic rstn,

inout logic kclk,
input logic kdata,
input logic [2:0] io_addr,
input logic io_w_en,

output logic[7:0] io_data_out,
output logic busy_flag,
output logic int_flag
);

localparam KEYBOARD_ID = 3'b001;

logic kclk_test;

logic kclkf, kdataf;
logic data_request;
logic [7:0] datacur;
logic [7:0] dataprev, dataprev_nxt;
logic [3:0] cnt;
logic cflag;

logic [15:0] scancode_pair, scancode_pair_nxt;
logic pflag, pflag_nxt;

logic shifting, shifting_nxt;
logic write_en, write_en_nxt;

logic [7:0] ascii;

logic busy_flag_nxt;
logic [7:0] data_buffer, data_buffer_nxt;

typedef enum {
    RECEIVE,
    PROCESSING,
    READY
} state_t;

state_t state, state_nxt;



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


assign data_request = (io_addr == KEYBOARD_ID) & ~io_w_en;

// FSM
always_ff @(posedge clk or negedge rstn) begin
    if(!rstn) begin
        state <= RECEIVE;
        pflag <= '0;
        dataprev <= '0;
        scancode_pair <= '0;
        shifting <= '0;
        write_en <= '0;
        data_buffer <= 8'h0;
    end else begin
        state <= state_nxt;

        pflag <= pflag_nxt;
        dataprev <= dataprev_nxt;
        scancode_pair <= scancode_pair_nxt;

        shifting <= shifting_nxt;
        write_en <= write_en_nxt;

        data_buffer <= data_buffer_nxt;
    end
end

always_comb begin
    state_nxt = state;

    pflag_nxt = pflag;
    dataprev_nxt = dataprev;
    scancode_pair_nxt = scancode_pair;

    shifting_nxt = shifting;

    data_buffer_nxt = data_buffer;

    case (state)
        RECEIVE: begin
            if (cflag == 1'b1 && pflag == 1'b0) begin
                scancode_pair_nxt = {dataprev, datacur};
                dataprev_nxt = datacur;

                state_nxt = PROCESSING;
            end

            pflag_nxt = cflag;
        end
        PROCESSING: begin
            priority casez(scancode_pair)
                16'hF012: begin
                    shifting_nxt = 0;
                    state_nxt = RECEIVE;
                end
                16'h??12: begin
                    shifting_nxt = 1;
                    state_nxt = RECEIVE;
                end
                16'hF0??: begin
                    data_buffer_nxt = ascii;
                    state_nxt = READY;
                end
                default: state_nxt = RECEIVE;
            endcase
        end
        READY: begin
            if(data_request)
                state_nxt = RECEIVE;
        end
    endcase
end

// Receiving scancodes
always_ff @(negedge kclkf) begin
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

//Outputs
always_ff @(posedge clk or negedge rstn) begin
    if(!rstn) begin
        busy_flag <= 1'b1;
    end else begin
        busy_flag <= busy_flag_nxt;
    end
end

assign io_data_out = data_request ? data_buffer : 8'hz;
assign int_flag = ~busy_flag;

always_comb begin
    busy_flag_nxt = busy_flag;

    case(state)
        RECEIVE: busy_flag_nxt = 1;
        PROCESSING: ;
        READY: begin
            busy_flag_nxt = 0;
            if(data_request)
                busy_flag_nxt = 1;
        end
    endcase
end

endmodule