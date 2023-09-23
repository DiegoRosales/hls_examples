#pragma once
#include "ap_fixed.h"

// Constants
static const int N = 1024;
static const int n_clog2_c = 10;

// Types
typedef ap_int<24> TI_INPUT_SIGNAL;
typedef ap_fixed<32, 2> TR_TWIDDLE_FACTOR;
typedef std::complex<TR_TWIDDLE_FACTOR> TC_TWIDDLE_FACTOR;
typedef ap_fixed<32, 24> TR_FFT;
typedef std::complex<TR_FFT> TC_FFT;
typedef ap_uint<n_clog2_c> TUI_SAMPLE_ARRAY_IDX;
