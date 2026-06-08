`timescale 1ns / 1ps
`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif

module top (
    input  logic        clk,
    input  logic        _reset,
    input  logic        rx,
    output logic        tx,
    output logic [3:0]  vga_r,
    output logic [3:0]  vga_g,
    output logic [3:0]  vga_b,
    output logic        h_sync,
    output logic        v_sync,
    output logic [15:0] led,
    output logic [ 7:0] seg,
    output logic [ 7:0] an,
    inout  logic        ps2_clk,
    input  logic        ps2_data
);
    logic clk_25 = 0;
    logic [1:0] clk_25_cnt = 0;
    always_ff @(posedge clk) begin
        clk_25_cnt <= clk_25_cnt + 1'b1;
        if (clk_25_cnt[0]) clk_25 <= ~clk_25;
    end

    logic clk_5 = 0;
    logic [3:0] clk_5_cnt = 0;
    always_ff @(posedge clk) begin
        if (clk_5_cnt == 4'd9) begin
            clk_5_cnt <= 0;
            clk_5 <= ~clk_5;
        end else begin
            clk_5_cnt <= clk_5_cnt + 1'b1;
        end
    end

    (* ASYNC_REG = "TRUE" *) logic [3:0] reset_sync;
    always_ff @(posedge clk) reset_sync <= {reset_sync[2:0], _reset};
    wire sys_rst_raw = reset_sync[3];

    (* ASYNC_REG = "TRUE" *) logic [2:0] sys_rst_5_sync;
    always_ff @(posedge clk_5) sys_rst_5_sync <= {sys_rst_5_sync[1:0], sys_rst_raw};
    wire sys_rst_5 = sys_rst_5_sync[2];

    (* ASYNC_REG = "TRUE" *) logic [2:0] sys_rst_25_sync;
    always_ff @(posedge clk_25) sys_rst_25_sync <= {sys_rst_25_sync[1:0], sys_rst_raw};
    wire sys_rst_25 = sys_rst_25_sync[2];

    logic [15:0] prog2mem_data, prog2mem_addr;
    logic prog2mem_w_en, prog2mem_stream_en;
    (* ASYNC_REG = "TRUE" *) logic [2:0] prog_stream_en_5_sync;
    always_ff @(posedge clk_5) prog_stream_en_5_sync <= {prog_stream_en_5_sync[1:0], prog2mem_stream_en};
    wire cpu_reset = sys_rst_5 || prog_stream_en_5_sync[2];

    logic [31:0] mem2core_instr;
    logic [15:0] core2mem_instr_pointer, mem2core_data_r, core2mem_addr, core2mem_data_w;
    logic core2mem_w_en;
    logic [7:0] io2core_int_f, io2core_busy_f;
    logic [15:0] io2core_data_r, core2io_data_w;
    logic [2:0] core2io_addr;
    logic core2io_w_en, core2io_r_en;

    core cpu (
        .clk(clk_5), ._reset(cpu_reset),
        .mem2core_instr(mem2core_instr), .core2mem_instr_pointer(core2mem_instr_pointer),
        .mem2core_data_r(mem2core_data_r), .core2mem_addr(core2mem_addr),
        .core2mem_data_w(core2mem_data_w), .core2mem_w_en(core2mem_w_en),
        .io2core_int_f(io2core_int_f), .io2core_busy_f(io2core_busy_f), .io2core_data_r(io2core_data_r),
        .core2io_addr(core2io_addr), .core2io_w_en(core2io_w_en), .core2io_r_en(core2io_r_en), .core2io_data_w(core2io_data_w)
    );

    logic [15:0] mem2gpu_data, gpu2mem_addr;
    memory_controller mem_ctrl (
        .clk_gpu(clk_25), .clk_cpu(clk_5), ._reset(sys_rst_25),
        .mem2core_instr(mem2core_instr), .mem2core_data_r(mem2core_data_r),
        .core2mem_instr_pointer(core2mem_instr_pointer), .core2mem_addr(core2mem_addr),
        .core2mem_data_w(core2mem_data_w), .core2mem_write_en(core2mem_w_en),
        .prog2mem_data(prog2mem_data), .prog2mem_addr(prog2mem_addr),
        .prog2mem_w_en(prog2mem_w_en), .prog2mem_stream_en(prog2mem_stream_en),
        .mem2gpu_data(mem2gpu_data), .gpu2mem_addr(gpu2mem_addr)
    );

    logic [2:0] gpu_red, gpu_green; logic [1:0] gpu_blue;
    logic gpu_int_f, gpu_busy_f;
    graphics_card gpu_inst (
        .clk(clk_25), ._reset(sys_rst_25),
        .red(gpu_red), .green(gpu_green), .blue(gpu_blue),
        .h_sync(h_sync), .v_sync(v_sync), .video_enable(),
        .gpu2io_int_f(gpu_int_f), .gpu2io_busy_f(gpu_busy_f),
        .io2gpu_w_en(core2io_addr == 3'b010 && core2io_w_en), .io2gpu_data_w(core2io_data_w),
        .mem2gpu_data(mem2gpu_data), .gpu2mem_addr(gpu2mem_addr)
    );
    assign vga_r = {gpu_red, 1'b0}; assign vga_g = {gpu_green, 1'b0}; assign vga_b = {gpu_blue, 2'b0};

    (* MARK_DEBUG = "true" *) logic kb_int_f, kb_busy_f;
    (* MARK_DEBUG = "true" *) logic [15:0] kb_data_r;
    (* MARK_DEBUG = "true" *) logic io2kb_r_en;
    ps2_keyboard_controller keyboard (
        .clk(clk_5), .rstn(~cpu_reset),
        .kclk(ps2_clk), .kdata(ps2_data),
        .io2kb_r_en(io2kb_r_en),
        .kb2io_data_r(kb_data_r), .kb2io_busy_f(kb_busy_f), .kb2io_int_f(kb_int_f)
    );

    (* MARK_DEBUG = "true" *) logic tim2io_int_f, tim2io_busy_f;
    (* MARK_DEBUG = "true" *) logic io2tim_r_en;
    timer #(.CLK_HZ(5_000_000), .PERIOD_US(1000)) system_timer (
        .clk(clk_5), ._reset(cpu_reset),
        .io2tim_r_en(io2tim_r_en),
        .tim2io_int_f(tim2io_int_f), .tim2io_busy_f(tim2io_busy_f)
    );

    io_bridge bridge_inst (
        .core2io_addr(core2io_addr), .core2io_w_en(core2io_w_en), .core2io_r_en(core2io_r_en), .core2io_data_w(core2io_data_w),
        .io2core_int_f(io2core_int_f), .io2core_busy_f(io2core_busy_f), .io2core_data_r(io2core_data_r),
        .tim2io_int_f(tim2io_int_f), .tim2io_busy_f(tim2io_busy_f), .tim2io_data_r(16'b0), .io2tim_r_en(io2tim_r_en),
        .kb2io_int_f(kb_int_f), .kb2io_busy_f(kb_busy_f), .kb2io_data_r(kb_data_r), .io2kb_r_en(io2kb_r_en),
        .gpu2io_int_f(gpu_int_f), .gpu2io_busy_f(gpu_busy_f), .io2gpu_w_en(), .io2gpu_data_w()
    );

    logic [7:0] uart_rx_data; logic uart_rx_empty, uart_rd;
    uart #(.DVSR(14), .DVSR_BIT(7)) prog_uart (.clk(clk_25), ._reset(sys_rst_25), .rd_uart(uart_rd), .wr_uart(1'b0), .rx(rx), .w_data(8'b0), .tx_full(), .rx_empty(uart_rx_empty), .tx(tx), .r_data(uart_rx_data));
    progmem programmer (.clk(clk_25), ._reset(sys_rst_25), .r_data(uart_rx_data), .rx_empty(uart_rx_empty), .rd_uart(uart_rd), .prog2mem_stream_en(prog2mem_stream_en), .prog2mem_write_en(prog2mem_w_en), .prog2mem_addr(prog2mem_addr), .prog2mem_data(prog2mem_data), .prog2crc_cg_f(), .prog2crc_rst_f());

    logic [19:0] seg_cnt = 0;
    always_ff @(posedge clk) seg_cnt <= seg_cnt + 1'b1;
    logic [3:0] hex_digit;
    always_comb begin
        case (seg_cnt[19:17])
            3'b000: hex_digit = core2mem_instr_pointer[3:0];
            3'b001: hex_digit = core2mem_instr_pointer[7:4];
            3'b010: hex_digit = core2mem_instr_pointer[11:8];
            3'b011: hex_digit = core2mem_instr_pointer[15:12];
            default: hex_digit = 4'h0;
        endcase
    end
    always_comb begin
        case (hex_digit)
            4'h0: seg = 8'b11000000; 4'h1: seg = 8'b11111001; 4'h2: seg = 8'b10100100; 4'h3: seg = 8'b10110000;
            4'h4: seg = 8'b10011001; 4'h5: seg = 8'b10010010; 4'h6: seg = 8'b10000010; 4'h7: seg = 8'b11111000;
            4'h8: seg = 8'b10000000; 4'h9: seg = 8'b10010000; 4'hA: seg = 8'b10001000; 4'hB: seg = 8'b10000011;
            4'hC: seg = 8'b11000110; 4'hD: seg = 8'b10100001; 4'hE: seg = 8'b10000110; 4'hF: seg = 8'b10001110;
            default: seg = 8'b11111111;
        endcase
    end
    always_comb begin
        an = 8'b11111111;
        if (seg_cnt[19:17] < 4) an[seg_cnt[19:17]] = 1'b0;
    end

    assign led[15] = seg_cnt[19];
    assign led[14] = keyboard.kclkf;
    assign led[13] = keyboard.kdataf;
    assign led[12:8] = core2mem_instr_pointer[4:0];
    assign led[7] = tim2io_int_f;
    assign led[6] = tim2io_busy_f;
    assign led[5:4] = 2'b0;
    assign led[3] = prog2mem_stream_en;
    assign led[2] = kb_busy_f;
    assign led[1] = kb_int_f;
    assign led[0] = cpu_reset;
endmodule
