############################################
## Integration script for the Zybo Sampler
############################################

set integ_script_dir [file normalize [file dirname [info script]]]

## Integrate design for synthesis
source ${integ_script_dir}/fft_demo_integ_synth.tcl
## Integrate design for simulation
#source ${integ_script_dir}/fft_demo_integ_sim.tcl

# source ${integ_script_dir}/bd_project.tcl

add_files -fileset constrs_1 -norecurse ./source/constraints/physical_constraints.xdc
add_files -fileset constrs_1 -norecurse ./source/constraints/pin_assignment.xdc
add_files -fileset constrs_1 -norecurse ./source/constraints/timing_constraints.xdc
set_property used_in_synthesis false [get_files  ./source/constraints/pin_assignment.xdc]
set_property used_in_synthesis false [get_files  ./source/constraints/physical_constraints.xdc]


make_wrapper -files [get_files ./target/fft_demo_integ/bd/fft_demo_top/fft_demo_top.bd] -top
add_files -norecurse ./target/fft_demo_integ/bd/fft_demo_top/hdl/fft_demo_top_wrapper.v

launch_runs impl_1 -to_step write_bitstream -jobs 2
wait_on_runs impl_1
write_hw_platform -fixed -include_bit -force -file ./target/fft_demo_integ/fft_demo_top_wrapper.xsa
write_hwdef -force -file ./target/fft_demo_integ/fft_demo_top_wrapper.hdf
