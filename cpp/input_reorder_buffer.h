/*
    File: input_reorder_buffer.h
    Author: Lucas Mariano Grigolato
    Date: 2024/02/25
    Description: This header file defines the 'input_reorder_buffer' class
    template, which facilitates the reordering of input samples for the Radix-2
    Fast Fourier Transform (FFT) algorithm. The class provides a method to store
    input samples from a stream into separate arrays representing the lower and
    upper halves of the FFT input. It reorders the samples to align with the
    requirements of the FFT algorithm, ensuring efficient computation. The class
    also includes methods for initialization and resetting of internal
    variables.
*/

#pragma once

#include <complex>
#include "fft_sysdef.h"
#include "hls_stream.h"

template <int N, int n_clog2_c>
class input_reorder_buffer
{
public:
    // Constructor
    input_reorder_buffer(void) {}

    // Destructor
    ~input_reorder_buffer(void) {}

    // Method to store input samples and reorder them for FFT computation
    void store_sample(
        // Inputs
        hls::stream<TI_INPUT_SIGNAL> &input_signal_stream, // Input stream of samples
        // Outputs
        TC_FFT fft_input_lower[N / 2], // Array for lower half of FFT input
        TC_FFT fft_input_upper[N / 2]) // Array for upper half of FFT input
    {
        TI_INPUT_SIGNAL input_sample;       // Input sample
        TUI_SAMPLE_ARRAY_IDX idx_reordered; // Index for reordered sample

        // Read samples from input channel until N samples are stored
        for (int i = 0; i < N; i++)
        {
            input_signal_stream.read(input_sample); // Read input sample
            idx_reordered = TUI_SAMPLE_ARRAY_IDX(i);               // Calculate reordered index
            idx_reordered.reverse();         // Reverse bits for reordering

            // Store sample in appropriate half based on reordered index
            if (idx_reordered.test(0))
            {
                // Odd indexes are stored in the upper half
                fft_input_upper[N / 2 - 1 - (idx_reordered >> 1)] = TC_FFT(TR_FFT(input_sample), 0);
            }
            else
            {
                // Even indexes are stored in the lower half
                fft_input_lower[idx_reordered >> 1] = TC_FFT(TR_FFT(input_sample), 0);
            }
        }
    }
};
