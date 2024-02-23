############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
## Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
############################################################
open_project vitis_project
set_top fft_wrapper
add_files ../../cpp/fft_wrapper.cpp
add_files -tb ../../cpp/fft_wrapper_test.cpp -cflags "-Wno-unknown-pragmas"
add_files -tb ../../cpp/input_reorder_buffer_wrapper.cpp -cflags "-Wno-unknown-pragmas"
add_files -tb ../../dat/file_example_WAV_1MG.dat -cflags "-Wno-unknown-pragmas"
open_solution "solution1" -flow_target vivado
set_part {xc7z020-clg400-1}
create_clock -period 10 -name default
config_cosim -tool xsim
source "./directives.tcl"
csim_design -clean
csynth_design
cosim_design -tool xsim
export_design -format ip_catalog
