module progmem #(
    parameter ADDR_WIDTH = 16
)(
    input logic clk,
    input logic _reset,
    input logic [7:0] r_data,     
    input logic rx_empty,   

    output logic rd_uart,    
    output logic prog2mem_stream_en,
    output logic prog2mem_write_en,
    output logic [ADDR_WIDTH-1:0] prog2mem_addr,
    output logic [15:0] prog2mem_data
);

typedef enum logic [1:0] {
    IDLE        = 2'b00,
    BYTE_LOW    = 2'b01,
    BYTE_HIGH   = 2'b10
} state_t;

state_t curr_state, next_state;

logic [ADDR_WIDTH-1:0] addr_reg, addr_next;
logic [7:0] low_byte_reg, low_byte_next;
logic prog2mem_stream_en_reg, prog2mem_stream_en_next;


always_ff @(posedge clk) begin
    if (!_reset) begin
        curr_state <= IDLE;
        addr_reg <= '0;
        low_byte_reg <= '0;
        prog2mem_stream_en_reg <= 1'b0;
    end else begin
        curr_state <= next_state;
        addr_reg <= addr_next;
        low_byte_reg <= low_byte_next;
        prog2mem_stream_en_reg <= prog2mem_stream_en_next;
    end
end


always_comb begin
    next_state = curr_state;
    addr_next = addr_reg;
    low_byte_next = low_byte_reg;
    prog2mem_stream_en_next = prog2mem_stream_en_reg;
    
    rd_uart = 1'b0;
    prog2mem_write_en = 1'b0;
    prog2mem_addr = addr_reg;

    prog2mem_data = {r_data, low_byte_reg}; 

    case (curr_state)
        IDLE: begin
            addr_next = '0;
            if (!rx_empty) begin
                prog2mem_stream_en_next = 1'b1;
                next_state     = BYTE_LOW;
            end else begin
                prog2mem_stream_en_next = 1'b0;
            end
        end

        BYTE_LOW: begin
            prog2mem_stream_en_next = 1'b1;
            if (!rx_empty) begin
                rd_uart        = 1'b1;
                low_byte_next  = r_data;
                next_state     = BYTE_HIGH;
            end
        end

        BYTE_HIGH: begin
            prog2mem_stream_en_next = 1'b1;
            if (!rx_empty) begin
                rd_uart   = 1'b1;
                prog2mem_write_en  = 1'b1;
                addr_next = addr_reg + 1'b1;
                next_state = BYTE_LOW;
            end
        end

        default: next_state = IDLE;
    endcase

    prog2mem_stream_en = prog2mem_stream_en_reg;
end

endmodule
