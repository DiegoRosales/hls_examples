#pragma once
#include "ap_fixed.h"
#include "hls_stream.h"
#include "fft_sysdef.h"
#include "input_reorder_buffer.h"

void input_reorder_buffer_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    TC_FFT fft_input_lower[N / 2],
    TC_FFT fft_input_upper[N / 2],
    TB &buffer_full);