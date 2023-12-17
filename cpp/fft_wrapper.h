#include "ap_fixed.h"
#include "hls_stream.h"
#include "fft_sysdef.h"
#include "input_reorder_buffer.h"
#include "fft.h"

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    TC_FFT fft_output[N],
    TB &valid);