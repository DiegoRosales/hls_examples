/*
    File: fft_wrapper.h
    Author: Lucas Mariano Grigolato
    Date: February 25, 2024
    Description: Header file containing declarations for the wrapper function of the Radix-2 FFT HLS block.
*/

#pragma once
#include "ap_fixed.h"
#include "hls_stream.h"
#include "fft_sysdef.h"
#include "input_reorder_buffer.h"
#include "fft.h"

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    TC_FFT fft_output_lower[N / 2],
    TC_FFT fft_output_upper[N / 2]);