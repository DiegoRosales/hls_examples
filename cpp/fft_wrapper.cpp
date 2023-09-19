#include "fft_wrapper.h"

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    TB &start,
    // Outputs
    TC_FFT fft_output[N],
    TB &done)
{
    // fprintf(stdout, "done = %d\n", done);
    // fprintf(stdout, "start = %d\n", start);
    static fft<N, n_clog2_c> fft_obj;
    fft_obj.computeFFTMagnitude(input_signal, start, fft_output, done);
}
