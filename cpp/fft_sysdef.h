#pragma once
#include "ap_fixed.h"

// Constexpr function to calculate the ceiling of the base-2 logarithm using recursion
constexpr unsigned int clog2_recursive(unsigned int n, unsigned int p = 0)
{
    return (n <= 1) ? p : clog2_recursive((n + 1) / 2, p + 1);
}

// Constants
static const int N = 256;
static const int n_clog2_c = clog2_recursive(N);

// Types
typedef ap_int<24> TI_INPUT_SIGNAL;
typedef ap_fixed<16, 2> TR_TWIDDLE_FACTOR;
typedef std::complex<TR_TWIDDLE_FACTOR> TC_TWIDDLE_FACTOR;
typedef ap_fixed<32, 24> TR_FFT;
typedef std::complex<TR_FFT> TC_FFT;
typedef ap_uint<n_clog2_c> TUI_SAMPLE_ARRAY_IDX;
typedef ap_uint<n_clog2_c + 1> TUI_SAMPLE_ARRAY_COUNT;
typedef ap_uint<1> TB;
