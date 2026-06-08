open_hw_manager
connect_hw_server -allow_non_jtag
open_hw_target

current_hw_device [get_hw_devices xc7a100t_0]
set_property PROGRAM.FILE {/home/szop/Projects/ALU/maluch/Hard/maluch-nexys4ddr/maluch-nexys4ddr.runs/impl_1/top.bit} [get_hw_devices xc7a100t_0]
set_property PROBES.FILE {/home/szop/Projects/ALU/maluch/Hard/maluch-nexys4ddr/maluch-nexys4ddr.runs/impl_1/top.ltx} [get_hw_devices xc7a100t_0]
set_property FULL_PROBES.FILE {/home/szop/Projects/ALU/maluch/Hard/maluch-nexys4ddr/maluch-nexys4ddr.runs/impl_1/top.ltx} [get_hw_devices xc7a100t_0]

puts "Programming device..."
program_hw_devices [get_hw_devices xc7a100t_0]
refresh_hw_device [lindex [get_hw_devices xc7a100t_0] 0]
puts "Programming successful!"
exit 0
