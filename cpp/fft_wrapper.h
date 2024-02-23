#pragma once
#include "ap_fixed.h"
#include "hls_stream.h"
#include "fft_sysdef.h"
#include "fft.h"

void fft_wrapper(
    // Inputs
    TC_FFT fft_input_lower[N / 2],
    TC_FFT fft_input_upper[N / 2],
    // Outputs
    TC_FFT fft_output[N]);