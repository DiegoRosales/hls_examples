############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
## Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
############################################################
set_directive_top -name fft_wrapper "fft_wrapper"
set_directive_interface -mode ap_ctrl_hs "fft_wrapper"
set_directive_interface -mode ap_memory "fft_wrapper" fft_input
set_directive_interface -mode s_axilite "fft_wrapper" fft_output
set_directive_disaggregate "fft_wrapper" fft_output
