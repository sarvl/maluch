`define PRECISE_SIM

`include "counter.sv"

`ifndef TYPES
    `include "types.sv"
    `define TYPES
`endif

`define JMP 16'b0100_1000_0000_0000
`define BEE 16'b0100_1001_0000_0000
`define BNE 16'b0100_1010_0000_0000
`define BGE 16'b0100_1011_0000_0000
`define BLE 16'b0100_1100_0000_0000
`define BLL 16'b0100_1101_0000_0000
`define BGG 16'b0100_1110_0000_0000
`define BOO 16'b0100_1111_0000_0000
`define BBS 16'b0101_1000_0000_0000
`define BSS 16'b0101_1001_0000_0000
`define BNS 16'b0101_1010_0000_0000
`define BAE 16'b0101_1011_0000_0000
`define BBE 16'b0101_1100_0000_0000
`define BAA 16'b0101_1101_0000_0000
`define BBB 16'b0101_1110_0000_0000
`define BNO 16'b0101_1111_0000_0000

`define RUNS 5

`timescale 1ns/10ps

module counter_tb();

    bit [15:0] test_code [16] = {
        `JMP, `BEE, `BNE, `BGE, `BLE, `BLL, `BGG, `BOO,
        `BBS, `BSS, `BNS, `BAE, `BBE, `BAA, `BBB, `BNO
    };

    string test_name = {
        "JMP", "BEE", "BNE", "BGE", "BLE", "BLL", "BGG", "BOO",
        "BBS", "BSS", "BNS", "BAE", "BBE", "BAA", "BBB", "BNO"
    };

    import types::csr_t;

    logic [31:0]  instr_pointer;
    csr_t         csr;
    logic [31:0]  instruction;
    logic [15:0]  src2;
    logic [31:0] _next_pointer;

    counter uut(.instr_pointer, .csr, .instruction, .src2, ._next_pointer);

    int all_succes = 0;
    int all_runs = 0;

    initial begin
        #0  instr_pointer = 32'h1;
            csr = 0;
            instruction = 0;
            src2 = 0;


        foreach (test_code[i]) begin
            int success_rate = 0;
            #10
            $display("--- %s%s%s ---", test_name[3*i], test_name[3*i+1], test_name[3*i+2]);
            for (int k=0; k<`RUNS; k++) begin
                bit [15:0] addr = $random() % 32000;
                instruction [31:16] = test_code[i];
                instruction [15:0 ] = addr;
                src2 = addr;

                #5 if (_next_pointer[31:16]==addr && _next_pointer[15:0] == addr+1) begin
                        success_rate++;
                    end
            end
            $display("  Sucess rate: %f %%", (success_rate/`RUNS * 100));
            $display("  Failed: %d", `RUNS-success_rate);
            all_succes += success_rate;
            all_runs += `RUNS;
        end

        #20
        $display("-----  SUMMARY  -----");
        $display("   Runs: %d ", all_runs);
        $display("   Commands: %d ", all_runs/`RUNS);
        $display("   Overal succes: %f %%", ($itor(all_succes)/$itor(all_runs) * 100));
        $display("   Overal fails: %d ", all_runs-all_succes);
        $display("   Command fails: %d ", (all_runs-all_succes)/`RUNS);
        $finish;
    end


    initial begin
        $dumpfile("waveforms/counter.fst");
        $dumpvars(0, counter_tb);
    end

endmodule
