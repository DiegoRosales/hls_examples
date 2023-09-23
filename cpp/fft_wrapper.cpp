#include "fft_wrapper.h"

void fft_wrapper(
    // Inputs
    TI_INPUT_SIGNAL input_signal[N],
    // Outputs
    TC_FFT fft_output[N])
{
    static fft<N, n_clog2_c> fft_obj;
    fft_obj.computeFFTMagnitude(input_signal, fft_output);
}
