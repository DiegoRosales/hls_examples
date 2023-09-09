############################################
## Integration script for the Zybo Sampler
############################################

set integ_script_dir [file normalize [file dirname [info script]]]

## Integrate design for synthesis
# source ${integ_script_dir}/fft_demo_integ_synth.tcl
## Integrate design for simulation
#source ${integ_script_dir}/fft_demo_integ_sim.tcl

source ${integ_script_dir}/bd_project.tcl

add_files -fileset constrs_1 -norecurse {/home/diego/proj/hls_examples/source/constraints/physical_constraints.xdc /home/diego/proj/hls_examples/source/constraints/pin_assignment.xdc /home/diego/proj/hls_examples/source/constraints/timing_constraints.xdc}
set_property used_in_synthesis false [get_files  /home/diego/proj/hls_examples/source/constraints/pin_assignment.xdc]
set_property used_in_synthesis false [get_files  /home/diego/proj/hls_examples/source/constraints/physical_constraints.xdc]
