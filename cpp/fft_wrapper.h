#include "ap_fixed.h"
#include "fft_sysdef.h"
#include "fft.h"
#include "hls_stream.h"

void fft_wrapper(
    // Inputs
    TI_INPUT_SIGNAL input_signal[N],
    // Outputs
    TC_FFT fft_output[N]);