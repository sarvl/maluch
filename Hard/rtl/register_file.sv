typedef struct packed {
    logic [7:0] int_flags;
    logic [7:0] busy_flags;
} regflags_t;

typedef struct packed {
    logic [7:0] int_mask;
    logic [7:0] rsvr;
} regmask_t;

module register_file #(
    parameter int DataWidth = 16,
    parameter int NumRegs = 16
)(
    // Control inputs
    input logic         clk,
    input logic         _reset,
    // R/W intpus
    input logic         reg_w_en,
    input logic         sp_w_en,
    input logic [15:0]  reg_in,
    input logic [15:0]  sp_in,
    input logic [3:0]   addr_in,
    input logic [3:0]   addr_out1,
    input logic [3:0]   addr_out2,
    // Outside inputs
    input logic [7:0]   int_flags,
    input logic [7:0]   busy_flags,
    // Outputs
    output logic [15:0] reg_out1,
    output logic [15:0] reg_out2
);

    regflags_t reg0;
    assign reg0.int_flags = int_flags;
    assign reg0.busy_flags = busy_flags;
    regmask_t reg1;
    logic [DataWidth-1:0] reg2;
    logic [DataWidth-1:0] regs[3:NumRegs-1];

    // Write logic
    always_ff @(posedge clk or negedge _reset) begin
        if (!_reset) begin
            for (int i=3; i<NumRegs; i++) begin
                regs[i] <= '0;
            end
            reg1 <= '0;
            reg2 <= '0;
        end else begin
            if (sp_w_en) begin
                reg2 <= sp_in;
            end
            if (reg_w_en) begin
                case (addr_in)
                    0: ;
                    1: reg1 <= reg_in;
                    2: reg2 <= reg_in;
                    default: regs[addr_in] <= reg_in;
                endcase
            end
        end
    end


    // Read logic
    always_comb begin
        case (addr_out1)
            0: reg_out1 = {reg0.int_flags & reg1.int_mask, reg0.busy_flags};
            1: reg_out1 = reg1;
            2: reg_out1 = reg2;
            default: reg_out1 = regs[addr_out1];
        endcase
        case (addr_out2)
            0: reg_out2 = {reg0.int_flags & reg1.int_mask, reg0.busy_flags};
            1: reg_out2 = reg1;
            2: reg_out2 = reg2;
            default: reg_out2 = regs[addr_out2];
        endcase
    end
endmodule
