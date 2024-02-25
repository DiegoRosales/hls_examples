/*
    File: fft_sysdef.h
    Author: Lucas Mariano Grigolato
    Date: 2024/02/25
    Description: Header file containing type and constant declarations for the
    Radix-2 FFT HLS block wrapper function.
*/

#pragma once
#include "ap_fixed.h"

// Constexpr function to calculate the ceiling of the base-2 logarithm using recursion
constexpr unsigned int clog2_recursive(unsigned int n, unsigned int p = 0)
{
    return (n <= 1) ? p : clog2_recursive((n + 1) / 2, p + 1);
}

// Constants
static const int N = 256;                        // Number of FFT points
static const int n_clog2_c = clog2_recursive(N); // clog2(N)

// Types
// Input signal represented as 24-bit signed integers
typedef ap_int<24> TI_INPUT_SIGNAL;
// Twiddle factors represented as 16-bit fixed-point with 2 bits for the integer part
typedef ap_fixed<16, 2> TR_TWIDDLE_FACTOR;
typedef std::complex<TR_TWIDDLE_FACTOR> TC_TWIDDLE_FACTOR;
// FFT values represented as 32-bit fixed-point with 24 bits for the integer part
typedef ap_fixed<32, 24> TR_FFT;
typedef std::complex<TR_FFT> TC_FFT;
// Indexes represented as unsigned integers
typedef ap_uint<n_clog2_c> TUI_SAMPLE_ARRAY_IDX;
typedef ap_uint<n_clog2_c + 1> TUI_SAMPLE_ARRAY_COUNT; // One more bit to represent N
// Boolean type
typedef ap_uint<1> TB;
