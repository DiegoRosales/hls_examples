############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
## Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
############################################################
open_component -reset vitis_project
set_top dma_codec_mux_wrapper
add_files ../../cpp/dma_codec_mux_wrapper.cpp
set_part {xc7z020-clg400-1}
create_clock -period 10 -name clk
config_cosim -tool xsim
source "./directives.tcl"
# csim_design -clean
csynth_design
# cosim_design -tool xsim
export_design -format ip_catalog
