// This file muxes the output stream from the axi dma with the
// output stream from the CODEC

#include "dma_codec_mux_wrapper.h"

void dma_codec_mux_wrapper(
    // Inputs
    hls::stream<TR_INPUT_SIGNAL> &stream_from_dma,
    hls::stream<TUI_CODEC_SIGNAL> &stream_from_codec,
    TB cr_mux_select,  // 0 = dma, 1 = codec
    // Outputs
    hls::stream<TR_INPUT_SIGNAL> &fft_output_stream) {
  TR_INPUT_SIGNAL data_from_dma;
  TUI_CODEC_SIGNAL data_from_codec;
  TB data_available_dma = 0;
  TB data_available_codec = 0;

  data_available_dma = stream_from_dma.read_nb(data_from_dma);
  data_available_codec = stream_from_codec.read_nb(data_from_codec);
  if (cr_mux_select == 0) {
    if (data_available_dma) {
      fft_output_stream.write(data_from_dma);
    }
  } else {
    if (data_available_codec) {
      fft_output_stream.write(TR_INPUT_SIGNAL(data_from_codec));
    }
  }
}
