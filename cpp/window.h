/*
    File: window.h
    Author: Lucas Mariano Grigolato
    Date: 2026/07/29
    Description: This header file defines the 'window' class
    template, which facilitates the application of a windowing function
    to input samples for the Radix-2 Fast Fourier Transform (FFT)
    algorithm. The class provides a method to multiply input samples by
    the corresponding window coefficients.
*/

#pragma once

#include "fft_sysdef.h"

template <int N>
class window {
 public:
  TUI_SAMPLE_ARRAY_IDX sample_counter_;

  // Constructor
  window(void) {
    reset();  // Call the reset method to initialize the object
  }

  // Destructor
  ~window(void) {}

  // Reset function
  void reset() {
    sample_counter_ = TUI_SAMPLE_ARRAY_IDX(0);  // Reset sample counter to zero
  }

  void apply_window(
      // Inputs
      TR_INPUT_SIGNAL input_sample,
      TR_WINDOW_COEFFICIENT window_coeffs[N / 2],
      // Outputs
      TR_FFT &fft_input) {
    // Apply windowing function to the input sample
    if (sample_counter_ == N / 2) {
      fft_input = TR_FFT(input_sample);
    } else {
      fft_input = (sample_counter_ < N / 2) ? TR_FFT(input_sample) * window_coeffs[sample_counter_]
                                            : TR_FFT(input_sample) * window_coeffs[N - sample_counter_];
    }

    // Increment sample counter and wrap around if necessary
    sample_counter_ = (sample_counter_ + 1) % N;
  }
};
