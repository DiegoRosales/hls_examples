# Top
set_directive_top -name fft_wrapper "fft_wrapper"
set_directive_inline -recursive "fft_wrapper"
set_directive_pipeline "fft_wrapper"

# Interfaces
set_directive_interface -mode axis -register_mode both -register "fft_wrapper" input_signal
set_directive_interface -mode s_axilite -clock ctrl_clk "fft_wrapper"
set_directive_interface -mode s_axilite -register "fft_wrapper" fft_output
set_directive_interface -mode ap_ctrl_hs "fft_wrapper"

# Loops
set_directive_unroll "fft::reset/INIT_TWIDDLE_FACTORS"
set_directive_unroll "fft::reset/INIT_SAMPLE_ARRAY"
set_directive_unroll "fft::reset/INIT_BIT_REVERSED_INDEXES"
set_directive_unroll "fft::computeFFTMagnitude/SHIFT_REGISTER"
set_directive_unroll "fft::computeFFTMagnitude/INIT_FFT_RESULTS_ARRAY"
set_directive_unroll "fft::computeFFTMagnitude/FFT_MAGNITUDE_CALC"

# Arrays
set_directive_disaggregate "fft_wrapper" fft_output
set_directive_array_partition -type complete -dim 1 "fft_wrapper" fft_output
