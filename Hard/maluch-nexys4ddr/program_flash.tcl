# 1. Generate memory configuration bin file from bitstream using standard SPIx1 mode
write_cfgmem -format bin -size 16 -interface SPIx1 -loadbit "up 0x00000000 /home/szop/Projects/ALU/maluch/Hard/maluch-nexys4ddr/maluch-nexys4ddr.runs/impl_1/top.bit" -file /home/szop/Projects/ALU/maluch/Hard/maluch-nexys4ddr/top.bin -force

# 2. Program the configuration flash memory
open_hw_manager
connect_hw_server -allow_non_jtag
open_hw_target

current_hw_device [get_hw_devices xc7a100t_0]
refresh_hw_device [lindex [get_hw_devices xc7a100t_0] 0]

# Create configuration memory object and configure properties
set my_cfgmem [create_hw_cfgmem -hw_device [get_hw_devices xc7a100t_0] -mem_dev [lindex [get_cfgmem_parts {s25fl128sxxxxxx0-spi-x1_x2_x4}] 0]]

set_property PROGRAM.ADDRESS_RANGE  {use_info} $my_cfgmem
set_property PROGRAM.FILES [list {/home/szop/Projects/ALU/maluch/Hard/maluch-nexys4ddr/top.bin}] $my_cfgmem
set_property PROGRAM.PRM_FILE {} $my_cfgmem
set_property PROGRAM.UNUSED_PIN_TERMINATION {pull-none} $my_cfgmem
set_property PROGRAM.BLANK_CHECK  0 $my_cfgmem
set_property PROGRAM.ERASE  1 $my_cfgmem
set_property PROGRAM.CFG_PROGRAM  1 $my_cfgmem
set_property PROGRAM.VERIFY  1 $my_cfgmem
set_property PROGRAM.CHECKSUM  0 $my_cfgmem

# Program configuration flash memory
startgroup 
create_hw_bitstream -hw_device [get_hw_devices xc7a100t_0] [get_property PROGRAM.HW_CFGMEM_BITFILE [get_hw_devices xc7a100t_0]]
program_hw_devices [get_hw_devices xc7a100t_0]
program_hw_cfgmem -hw_cfgmem $my_cfgmem
endgroup

# Boot FPGA from flash configuration
boot_hw_device [get_hw_devices xc7a100t_0]
exit 0
