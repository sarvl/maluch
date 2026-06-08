open_hw_manager
connect_hw_server -allow_non_jtag
open_hw_target
current_hw_device [get_hw_devices xc7a100t_0]
refresh_hw_device [lindex [get_hw_devices xc7a100t_0] 0]

# Trigger the ILA immediately to read the current state of the nets
run_hw_ila [get_hw_ilas -of_objects [get_hw_devices xc7a100t_0] -filter {CELL_NAME=~"u_ila_0"}] -trigger_now
wait_on_hw_ila [get_hw_ilas -of_objects [get_hw_devices xc7a100t_0] -filter {CELL_NAME=~"u_ila_0"}]

# Get captured values from the ILA
# Since we triggered immediately, we can inspect the values
set data [upload_hw_ila_data [get_hw_ilas -of_objects [get_hw_devices xc7a100t_0] -filter {CELL_NAME=~"u_ila_0"}]]

# Print the status of key signals
puts "Captured ILA Data. Let's inspect the signals:"
exit 0
