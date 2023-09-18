#include "fft_wrapper.h"

void fft_wrapper(
    // Inputs
    hls::stream<TR_INPUT_SIGNAL> &input_signal,
    TB &start,
    // Outputs
    TR_FFT_NORM fft_magnitudes[N],
    TB &done)
{
    // fprintf(stdout, "done = %d\n", done);
    // fprintf(stdout, "start = %d\n", start);
    static fft<N, n_clog2_c> fft_obj;
    fft_obj.computeFFTMagnitude(input_signal, start, fft_magnitudes, done);
}
