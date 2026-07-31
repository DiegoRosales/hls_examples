set_directive_interface fft_wrapper -mode ap_ctrl_none
set_directive_interface fft_wrapper -mode axis -register_mode both input_signal_stream -register=true
set_directive_interface fft_wrapper -mode axis -register_mode both fft_output_stream -register=true
set_directive_interface fft_wrapper -mode s_axilite -bundle axi4l_if -clock clk return
set_directive_interface fft_wrapper -mode s_axilite -bundle axi4l_if -clock clk window_coeffs
set_directive_array_partition fft_wrapper -type complete -dim 1 fft_obj.fft_stage_lower
set_directive_array_partition fft_wrapper -type complete -dim 1 fft_obj.fft_stage_upper
set_directive_array_partition fft_wrapper -type complete -dim 1 fft_obj.precomputed_idx
set_directive_array_partition fft_wrapper -type complete -dim 1 fft_obj.twiddle_factors
set_directive_aggregate fft_wrapper -compact auto fft_obj.precomputed_idx
set_directive_aggregate fft_wrapper -compact auto fft_obj.twiddle_factors