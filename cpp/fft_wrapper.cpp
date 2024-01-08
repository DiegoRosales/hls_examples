#include "fft_wrapper.h"

void fft_wrapper(
    // Inputs
    TC_FFT fft_input[N],
    // Outputs
    TC_FFT fft_output[N])
{
    static fft<N, n_clog2_c> fft_obj;

    // Store sample in buffer
    fft_obj.doFFT(fft_input, fft_output);
}
