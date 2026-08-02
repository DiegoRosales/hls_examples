open_project ./fft_project.xpr
update_compile_order -fileset sources_1
launch_runs impl_1 -to_step write_bitstream -jobs 10
wait_on_run impl_1

open_bd_design [get_files fft_project_wrapper.bd]
write_bd_tcl -force ./fft_project_wrapper.tcl
