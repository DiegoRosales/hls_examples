/*
    File: fft_wrapper.cpp
    Author: Lucas Mariano Grigolato
    Date: 2024/02/25
    Description: This file contains a wrapper function for the Radix-2 FFT HLS
    block. The wrapper function instantiates two objects:
        - "input_reorder_buffer_obj": This object maps the incoming samples from
          the AXI4-Stream input interface into two memories of size N/2 (where N
          is the number of FFT points), in a specific order optimized for
          running the FFT algorithm with an initiation interval (II) of 1.
        - "fft_obj": This object receives the required samples to compute the
          N-point Radix-2 FFT, which are mapped into two separate memories to
          achieve an II of 1 in calculating each FFT value. The output is stored
          in two arrays of size N/2, which will be mapped to two memories in
          hardware.
*/

#include "fft_wrapper.h"

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    hls::stream<TC_FFT> &fft_output)
{
    // Objects instantiation
    static fft<N, n_clog2_c> fft_obj;
    static input_reorder_buffer<N, n_clog2_c> input_reorder_buffer_obj;

    // FFT input arrays
    TC_FFT fft_input_lower[N / 2];
    TC_FFT fft_input_upper[N / 2];

    // Store samples into FFT input memories
    input_reorder_buffer_obj.store_sample(
        // Inputs
        input_signal,
        // Outputs
        fft_input_lower,
        fft_input_upper);

    // Calculate the FFT
    fft_obj.doFFT(
        // Inputs
        fft_input_lower,
        fft_input_upper,
        // Outputs
        fft_output);
}
