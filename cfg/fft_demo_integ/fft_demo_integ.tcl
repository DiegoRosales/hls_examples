############################################
## Integration script for the Zybo Sampler
############################################

set integ_script_dir [file normalize [file dirname [info script]]]

set project_name         $integ_project_name
set project_dir          $integ_project_dir
set project_top          $integ_project_top
set ip_repo_list         [list  ${packaged_cores_dirname} \
                                ${user_interfaces_dir} \
                                ./hls/fft_wrapper/hls_project/solution_1/impl/ip \
                                ]
set bus_definition_list  [list ${vivado_interface_path}/gpio_v1_0/gpio.xml           \
                               ${vivado_interface_path}/axis_v1_0/axis.xml           \
                               ${vivado_interface_path}/aximm_v1_0/aximm.xml         \
                               ${vivado_interface_path}/interrupt_v1_0/interrupt.xml \
                               ${vivado_interface_path}/clock_v1_0/clock.xml         \
                               ${vivado_interface_path}/reset_v1_0/reset.xml         \
                               ${user_interfaces_dir}/i2s/i2s.xml                    \
                               ]

## Integrate design for synthesis
# source ${integ_script_dir}/fft_demo_integ_synth.tcl
## Integrate design for simulation
#source ${integ_script_dir}/fft_demo_integ_sim.tcl

# source ${integ_script_dir}/bd_project.tcl
if {[file exists $project_dir]} {
    exec rm -rf $project_dir
}
create_project $project_name $project_dir -part $FPGA_PART_NUMBER -force

set_property ip_repo_paths [list ./target/packaged_cores ./board_files/interfaces ./hls/fft_wrapper/hls_project/solution_1/impl/ip] [current_project]

integ_utils::load_bus_def $bus_definition_list

set_property board_part $BOARD_PART_NUMBER [current_project]

update_ip_catalog

# create_bd_design $project_name

source ${integ_script_dir}/bd_project.tcl
write_bd_tcl -force ./cfg/fft_demo_integ/bd_project.tcl

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
