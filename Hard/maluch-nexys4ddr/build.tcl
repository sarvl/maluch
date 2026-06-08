open_project /home/szop/Projects/ALU/maluch/Hard/maluch-nexys4ddr/maluch-nexys4ddr.xpr
update_compile_order -fileset sources_1
reset_run synth_1
launch_runs impl_1 -to_step write_bitstream -jobs 16
wait_on_run impl_1

# Check if implementation succeeded
set run_status [get_property STATUS [get_runs impl_1]]
puts "Build status: $run_status"
if {$run_status != "write_bitstream Complete!"} {
    puts "ERROR: Build failed!"
    exit 1
}
puts "SUCCESS: Bitstream generated!"
exit 0
