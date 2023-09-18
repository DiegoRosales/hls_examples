#pragma once
#include "ap_fixed.h"

static const int N = 1024;
static const int n_clog2_c = 10;
typedef ap_fixed<16, 1> TR_INPUT_SIGNAL;
typedef ap_fixed<24, 8> TR_TWIDDLE_FACTOR;
typedef std::complex<TR_TWIDDLE_FACTOR> TC_TWIDDLE_FACTOR;
typedef ap_ufixed<48, 16> TR_FFT;
typedef std::complex<TR_FFT> TC_FFT;
typedef ap_ufixed<48, 16> TR_FFT_NORM;
typedef ap_uint<1> TB;
typedef ap_uint<n_clog2_c> TUI_SAMPLE_ARRAY_IDX;