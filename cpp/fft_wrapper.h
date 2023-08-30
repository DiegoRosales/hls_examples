#include "ap_fixed.h"
#include "fft_sysdef.h"
#include "fft.h"
#include "hls_stream.h"

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    TC_FFT fft_output[N]);