// This file muxes the output stream from the axi dma with the
// output stream from the CODEC

#pragma once
#include "ap_fixed.h"
#include "hls_stream.h"
#include "fft_sysdef.h"
#include "input_reorder_buffer.h"
#include "fft.h"

void dma_codec_mux_wrapper(
    // Inputs
    hls::stream<TR_INPUT_SIGNAL> &stream_from_dma,
    hls::stream<TUI_CODEC_SIGNAL> &stream_from_codec,
    TB cr_mux_select,  // 0 = dma, 1 = codec
    // Outputs
    hls::stream<TR_INPUT_SIGNAL> &fft_output_stream);