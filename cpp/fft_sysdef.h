/*
    File: fft_sysdef.h
    Author: Lucas Mariano Grigolato
    Date: 2024/02/25
    Description: Header file containing type and constant declarations for the
    Radix-2 FFT HLS block wrapper function.
*/

#pragma once
#include "ap_fixed.h"
#include "ap_axi_sdata.h"

// Constexpr function to calculate the ceiling of the base-2 logarithm using recursion
constexpr unsigned int clog2_recursive(unsigned int n, unsigned int p = 0) {
  return (n <= 1) ? p : clog2_recursive((n + 1) / 2, p + 1);
}

// Constants
static const int N = 256;                         // Number of FFT points
static const int n_clog2_c = clog2_recursive(N);  // clog2(N)

// Types
// Input signal represented as 32-bit fixed-point with 2 bits for the integer part
typedef ap_fixed<32,2> TR_INPUT_SIGNAL;

// Window coefficients represented as 18-bit fixed-point with 0 bits for the integer part
typedef ap_ufixed<18, 0> TR_WINDOW_COEFFICIENT;

// Twiddle factors represented as 18-bit fixed-point with 2 bits for the integer part
typedef ap_fixed<18, 2> TR_TWIDDLE_FACTOR;
typedef std::complex<TR_TWIDDLE_FACTOR> TC_TWIDDLE_FACTOR;

// FFT values represented as 32-bit fixed-point with 10 bits for the integer part,
// giving 2x headroom over the worst-case unscaled DFT sum (N * max|input|).
// Convergent rounding (rather than the default truncation) avoids a biased
// error accumulating across each of the n_clog2_c butterfly stages.
typedef ap_fixed<32, 10, AP_RND_CONV, AP_SAT> TR_FFT;
typedef std::complex<TR_FFT> TC_FFT;
typedef hls::axis<TC_FFT> TC_FFT_OUTPUT;

// Indexes represented as unsigned integers
typedef ap_uint<n_clog2_c> TUI_SAMPLE_ARRAY_IDX;

// Boolean type
typedef ap_uint<1> TB;
