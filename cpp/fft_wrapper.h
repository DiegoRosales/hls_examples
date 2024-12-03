/*
    File: fft_wrapper.h
    Author: Lucas Mariano Grigolato
    Date: 2024/02/25
    Description: Header file containing declarations for the wrapper function of
    the Radix-2 FFT HLS block.
*/

#pragma once
#include "ap_fixed.h"
#include "hls_stream.h"
#include "fft_sysdef.h"
#include "input_reorder_buffer.h"
#include "fft.h"

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal_stream,
    // Outputs
    hls::stream<TC_FFT_OUTPUT> &fft_output_stream);