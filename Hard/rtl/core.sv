`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif
`include "alu.sv"
`include "control_unit.sv"
`include "register_file.sv"
`include "program_counter.sv"

/* verilator lint_off MULTITOP */
module core(
    input logic clk,
    input logic _reset,

    input logic [31:0] mem2core_instr,
    output logic [15:0] core2mem_instr_pointer,

    input logic [15:0] mem2core_data_r,
    output logic [15:0] core2mem_addr,
    output logic [15:0] core2mem_data_w,
    output logic core2mem_w_en,

    input logic [7:0] io2core_int_f,
    input logic [7:0] io2core_busy_f,
    input logic [15:0] io2core_data_r,
    output logic [2:0] core2io_addr,
    output logic core2io_w_en,
    output logic core2io_r_en,
    output logic [15:0] core2io_data_w
);

    localparam logic [15:0] IRQ_MICROCODE = 16'hC800; //1100 1000 0000 0000

    logic [31:0]    instruction /* verilator public */;
    logic [31:0]    int_instr;
    logic [15:0]    instr_pointer;
    logic [15:0]    _next_pointer;

    logic [15:0]    src1;
    logic [15:0]    src2;
    logic [15:0]    alu_ret;
    logic [2:0]     alu_ctrl;

    logic [15:0]    reg_out1;
    logic [15:0]    reg_out2;
    logic [15:0]    reg_in;
    logic [3:0]     addr_in;
    logic [3:0]     addr_out1;
    logic [3:0]     addr_out2;
    logic           reg_w_en;
    logic           sp_w_en;
    logic [15:0]    sp_in;
    logic           csr_flags_we;

    logic irq_f;
    logic iret_f;
    logic [3:0] irq_relative_addr;
    logic [7:0] masked_int_flags;

    logic [15:0]    instr_pointer_seq;
    logic [15:0]    instr_pointer_ctrl;

    csr_t csr;
    csr_t _csr_next;
    csr_t csr_buf;

    // Driving outputs
    always_ff @(posedge clk) instr_pointer <= _reset ? 0 : 
                                                irq_f ? {12'hfff, irq_relative_addr} : _next_pointer;
    
    always_ff @(posedge clk) int_instr <= {IRQ_MICROCODE, _next_pointer};

    // Driving csr
    always_ff @(posedge clk) begin
        if (csr_flags_we) csr <= _csr_next;
        else if (irq_f) csr_buf <= csr;
        else if (iret_f) csr <= csr_buf;
    end

    program_counter IP(
        .instr_pointer(instr_pointer),
        .csr(csr),
        .instruction(instruction),
        .instr_pointer_ctrl(instr_pointer_ctrl),
        
        .instr_pointer_seq(instr_pointer_seq),
        ._nxt_instr_pointer(_next_pointer)
    );
    control_unit CONTROL_UNIT(
        .instruction(instruction),
        .instr_pointer_seq(instr_pointer_seq),

        .csr_flags_we(csr_flags_we),
        .instr_pointer_ctrl(instr_pointer_ctrl),
        .iret_f(iret_f),

        // ALU
        .alu_ret(alu_ret),
        .src1(src1),
        .src2(src2),
        .alu_ctrl(alu_ctrl),

        // Registers
        .reg_out1(reg_out1),
        .reg_out2(reg_out2),
        .reg_in(reg_in),
        .addr_in(addr_in),
        .addr_out1(addr_out1),
        .addr_out2(addr_out2),
        .reg_w_en(reg_w_en),
        .sp_w_en(sp_w_en),
        .sp_in(sp_in),

        // IO
        .io_data_r(io2core_data_r),
        .io_addr(core2io_addr),
        .io_w_en(core2io_w_en),
        .io_r_en(core2io_r_en),
        .io_data_w(core2io_data_w),

        // Memory controller
        .mem_ctrl_data_r(mem2core_data_r),
        .mem_ctrl_addres(core2mem_addr),
        .mem_ctrl_data_w(core2mem_data_w),
        .mem_ctrl_write_en(core2mem_w_en)
    );
    alu ALU(
        .alu_ctrl(alu_ctrl),
        .src1(src1),
        .src2(src2),

        .alu_ret(alu_ret),
        ._csr_next(_csr_next)
    );
    register_file register(
        .clk(clk),
        ._reset(_reset),
        .reg_w_en(reg_w_en),
        .sp_w_en(sp_w_en),
        .reg_in(reg_in),
        .sp_in(sp_in),
        .addr_in(addr_in),
        .addr_out1(addr_out1),
        .addr_out2(addr_out2),

        .int_flags(io2core_int_f),
        .busy_flags(io2core_busy_f),
        .irq_f(irq_f),
        .iret_f(iret_f),

        .masked_int_flags(masked_int_flags),
        .reg_out1(reg_out1),
        .reg_out2(reg_out2)
    );
    interrupt_controller INT_CTRL(
        .clk(clk),
        ._reset(_reset),
        .masked_int_flags(masked_int_flags),
        .iret_f(iret_f),

        .irq_f(irq_f),
        .irq_relative_addr(irq_relative_addr)
    );

    assign core2mem_instr_pointer = instr_pointer;
    assign instruction = irq_f ? int_instr : mem2core_instr;

endmodule
