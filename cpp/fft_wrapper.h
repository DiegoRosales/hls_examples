#include "ap_fixed.h"
#include "fft_sysdef.h"
#include "fft.h"
#include "hls_stream.h"

void fft_wrapper(
    // Inputs
    hls::stream<TR_INPUT_SIGNAL> &input_signal,
    TB &start,
    // Outputs
    TR_FFT_NORM fft_magnitudes[N],
    TB &done);