#pragma once
#include "ap_fixed.h"

const int N = 32;
const int n_log_2 = 5;

void average_wrapper(
    // Inputs
    ap_uint<16> inputs[N],
    // Outputs
    ap_ufixed<24, 16> &average_result);