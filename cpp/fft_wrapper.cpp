#include "fft_wrapper.h"

void fft_wrapper(
    // Inputs
    TC_FFT fft_input_lower[N / 2],
    TC_FFT fft_input_upper[N / 2],
    // Outputs
    TC_FFT fft_output[N])
{
    static fft<N, n_clog2_c> fft_obj;

    TC_FFT fft_output_lower[N / 2];
    TC_FFT fft_output_upper[N / 2];

    fft_obj.doFFT(
        // Inputs
        fft_input_lower,
        fft_input_upper,
        // Outputs
        fft_output_lower,
        fft_output_upper);

OUTPUT_MAPPING_LOOP_LOWER:
    for (int i = 0; i < N / 2; i++)
    {
        fft_output[i] = fft_output_lower[i];
    }
OUTPUT_MAPPING_LOOP_UPPER:
    for (int i = N / 2; i < N; i++)
    {
        fft_output[i] = fft_output_upper[i - N / 2];
    }
}
