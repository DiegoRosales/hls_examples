#include "average_wrapper.h"

void average_wrapper(
    // Inputs
    ap_uint<16> inputs[N],
    // Outputs
    ap_ufixed<24, 16> &average_result)
{
    average_result = 0; // Reset accumulator

ACCUM_LOOP:
    for (int i = 0; i < N; i++)
    {
        average_result += inputs[i]; // Accumulate inputs
    }
    average_result = average_result >> n_log_2; // Divide by Log2(N)
}
