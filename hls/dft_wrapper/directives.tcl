set_directive_interface -mode s_axilite -bundle dft_wrapper  "dft_wrapper"
set_directive_interface -mode s_axilite -bundle dft_wrapper  "dft_wrapper" dft_magnitudes

set_directive_interface -mode axis    -register -register_mode both "dft_wrapper" input_signal
