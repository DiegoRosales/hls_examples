#include "ap_fixed.h"
#include "hls_stream.h"
#include "fft_sysdef.h"
#include "input_reorder_buffer.h"
#include "fft.h"

void fft_wrapper(
    // Inputs
    TC_FFT fft_input[N],
    // Outputs
    TC_FFT fft_output[N]);