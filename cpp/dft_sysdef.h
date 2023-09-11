#pragma once
#include "ap_fixed.h"

static const int N = 1024;
static const int n_clog2_c = 10;
typedef ap_fixed<16, 1> TR_INPUT_SIGNAL;
typedef ap_fixed<16, 1> TR_TWIDDLE_FACTOR;
typedef std::complex<TR_TWIDDLE_FACTOR> TC_TWIDDLE_FACTOR;
typedef ap_fixed<n_clog2_c * 2, n_clog2_c> TR_DFT;
typedef std::complex<TR_DFT> TC_DFT;
typedef ap_ufixed<n_clog2_c * 2, n_clog2_c> TR_DFT_NORM;
typedef ap_uint<1> TB;
