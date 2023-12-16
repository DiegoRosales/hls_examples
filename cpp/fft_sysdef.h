#pragma once
#include "ap_fixed.h"

// Constants
static const int n_instances_c = 4;
static const int N = 256;
static const int n_clog2_c = 8;

// Types
typedef ap_int<24> TI_INPUT_SIGNAL;
typedef ap_fixed<16, 2> TR_TWIDDLE_FACTOR;
typedef std::complex<TR_TWIDDLE_FACTOR> TC_TWIDDLE_FACTOR;
typedef ap_fixed<32, 24> TR_FFT;
typedef std::complex<TR_FFT> TC_FFT;
typedef ap_uint<n_clog2_c + 2> TUI_SAMPLE_ARRAY_IDX;
typedef ap_uint<1> TB;