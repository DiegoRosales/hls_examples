set_directive_interface dma_codec_mux_wrapper -mode ap_ctrl_none
set_directive_interface dma_codec_mux_wrapper -mode axis -register_mode both stream_from_dma -register=true
set_directive_interface dma_codec_mux_wrapper -mode axis -register_mode both stream_from_codec -register=true
set_directive_interface dma_codec_mux_wrapper -mode axis -register_mode both fft_output_stream -register=true
set_directive_interface dma_codec_mux_wrapper -mode s_axilite -bundle axi4l_if -clock clk return
set_directive_interface dma_codec_mux_wrapper -mode s_axilite -bundle axi4l_if -clock clk cr_mux_select

set_directive_pipeline II=1