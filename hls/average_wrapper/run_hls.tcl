############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
## Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
############################################################
open_project vitis_project
set_top average_wrapper
add_files ../../cpp/average_wrapper.cpp
add_files -tb ../../cpp/average_wrapper_test.cpp -cflags "-Wno-unknown-pragmas"
open_solution "solution_kept_rolled" -flow_target vivado
set_part {xc7z020-clg400-1}
create_clock -period 10 -name default
config_cosim -tool xsim
source "./directives_kept_rolled.tcl"
csim_design -clean
csynth_design
open_solution "solution_full_unrolling" -flow_target vivado
set_part {xc7z020-clg400-1}
create_clock -period 10 -name default
config_cosim -tool xsim
source "./directives_full_unrolling.tcl"
csynth_design
open_solution "solution_array_partition" -flow_target vivado
set_part {xc7z020-clg400-1}
create_clock -period 10 -name default
config_cosim -tool xsim
source "./directives_array_partition.tcl"
csynth_design
open_solution "solution_s_axilite" -flow_target vivado
set_part {xc7z020-clg400-1}
create_clock -period 10 -name default
config_cosim -tool xsim
source "./directives_s_axilite.tcl"
csynth_design
open_solution "solution_kept_rolled" -flow_target vivado