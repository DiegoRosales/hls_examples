############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
## Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
############################################################
set_directive_top -name fft_wrapper "fft_wrapper"
set_directive_dataflow "fft_wrapper"
set_directive_interface -mode ap_ctrl_chain "fft_wrapper"
set_directive_interface -mode axis -register_mode both -register=true "fft_wrapper" input_signal_stream
set_directive_interface -mode axis -register_mode both -register=true "fft_wrapper" fft_output_stream
set_directive_interface -mode s_axilite -bundle axi4l_if -clock clk -register=true "fft_wrapper"
set_directive_array_partition -type complete -dim 1 "fft_wrapper" fft_obj.fft_stage_lower
set_directive_array_partition -type complete -dim 1 "fft_wrapper" fft_obj.fft_stage_upper
set_directive_array_partition -type complete -dim 1 "fft_wrapper" fft_obj.precomputed_idx
set_directive_aggregate -compact auto "fft_wrapper" fft_obj.precomputed_idx
set_directive_aggregate -compact auto "fft_wrapper" fft_obj.twiddle_factors
