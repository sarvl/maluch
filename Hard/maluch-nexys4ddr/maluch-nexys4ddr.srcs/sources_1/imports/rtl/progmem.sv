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
    output logic [15:0] prog2mem_data,
    output logic prog2crc_cg_f,
    output logic prog2crc_rst_f
);

typedef enum logic [2:0] {
    IDLE      = 3'b000,
    BYTE_LOW  = 3'b001,
    BYTE_HIGH = 3'b010,
    PRE_WRITE = 3'b011,
    WRITE     = 3'b100,
    POST_WRITE= 3'b101
} state_t;

state_t curr_state, next_state;

logic [ADDR_WIDTH-1:0] addr_reg, addr_next;
logic [7:0] low_byte_reg, low_byte_next;
logic [7:0] high_byte_reg, high_byte_next;
logic prog2mem_stream_en_reg, prog2mem_stream_en_next;
logic [23:0] timeout_cnt; // Increased timeout to 24 bits (~670ms at 25MHz)

always_ff @(posedge clk) begin
    if (_reset) begin
        curr_state <= IDLE;
        addr_reg <= '0;
        low_byte_reg <= '0;
        high_byte_reg <= '0;
        prog2mem_stream_en_reg <= 1'b0;
        timeout_cnt <= '0;
    end else begin
        curr_state <= next_state;
        addr_reg <= addr_next;
        low_byte_reg <= low_byte_next;
        high_byte_reg <= high_byte_next;
        prog2mem_stream_en_reg <= prog2mem_stream_en_next;
        
        if (curr_state == IDLE) timeout_cnt <= '0;
        else if (rx_empty) timeout_cnt <= timeout_cnt + 1'b1;
        else timeout_cnt <= '0;
    end
end

always_comb begin
    next_state = curr_state;
    addr_next = addr_reg;
    low_byte_next = low_byte_reg;
    high_byte_next = high_byte_reg;
    prog2mem_stream_en_next = prog2mem_stream_en_reg;
    
    rd_uart = 1'b0;
    prog2mem_write_en = 1'b0;
    prog2mem_addr = addr_reg;
    prog2mem_data = {low_byte_reg, high_byte_reg};
    
    prog2crc_cg_f  = (curr_state != IDLE);
    prog2crc_rst_f = (curr_state != IDLE);

    case (curr_state)
        IDLE: begin
            addr_next = '0;
            if (!rx_empty) begin
                prog2mem_stream_en_next = 1'b1;
                next_state = BYTE_LOW;
            end else begin
                prog2mem_stream_en_next = 1'b0;
            end
        end

        BYTE_LOW: begin
            prog2mem_stream_en_next = 1'b1;
            if (timeout_cnt[23]) begin
                next_state = IDLE;
                prog2mem_stream_en_next = 1'b0;
            end else if (!rx_empty) begin
                rd_uart = 1'b1;
                low_byte_next = r_data;
                next_state = BYTE_HIGH;
            end
        end

        BYTE_HIGH: begin
            if (timeout_cnt[23]) begin
                next_state = IDLE;
                prog2mem_stream_en_next = 1'b0;
            end else if (!rx_empty) begin
                rd_uart = 1'b1;
                high_byte_next = r_data;
                next_state = PRE_WRITE;
            end
        end

        PRE_WRITE: begin
            next_state = WRITE;
        end

        WRITE: begin
            prog2mem_write_en = 1'b1;
            next_state = POST_WRITE;
        end

        POST_WRITE: begin
            addr_next = addr_reg + 1'b1;
            next_state = BYTE_LOW;
        end

        default: next_state = IDLE;
    endcase
end

assign prog2mem_stream_en = prog2mem_stream_en_reg;

endmodule
