module io_bridge(
    //Core
    input logic [2:0]   core2io_addr,
    input logic         core2io_w_en,
    input logic         core2io_r_en,
    input logic [15:0]  core2io_data_w,
    output logic [7:0]  io2core_int_f,
    output logic [7:0]  io2core_busy_f,
    output logic [15:0] io2core_data_r,
    //Timer
    input logic         tim2io_int_f,
    input logic         tim2io_busy_f,
    input logic[15:0]   tim2io_data_r,
    output logic        io2tim_r_en,
    //Keyboard
    input logic         kb2io_int_f,
    input logic         kb2io_busy_f,
    input logic[15:0]   kb2io_data_r,
    output logic        io2kb_r_en,
    //GPU
    input logic         gpu2io_int_f,
    input logic         gpu2io_busy_f,
    output logic        io2gpu_w_en,
    output logic [15:0] io2gpu_data_w
);

assign io2core_int_f = {tim2io_int_f, kb2io_int_f, gpu2io_int_f, 5'b0};
assign io2core_busy_f = {tim2io_busy_f, kb2io_busy_f, gpu2io_busy_f, 5'b0};

// Drive data from core
always_comb begin
    io2tim_r_en = 0;
    io2kb_r_en = 0;
    io2gpu_w_en = 0;
    io2gpu_data_w = 0;
    
    case (core2io_addr)
        3'b000: begin
            io2tim_r_en = core2io_r_en;
        end
        3'b001: begin
            io2kb_r_en = core2io_r_en;
        end
        3'b010: begin
            io2gpu_w_en = core2io_w_en;
            io2gpu_data_w = core2io_data_w;
        end
        default: ;
    endcase
end

// Drive data to core
always_comb begin
    io2core_data_r = 0;

    case (core2io_addr)
        3'b000: begin
            io2core_data_r = tim2io_data_r;
        end
        3'b001: begin
            io2core_data_r = kb2io_data_r;
        end
        default: ;
    endcase
end

endmodule