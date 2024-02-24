#include "fft_wrapper.h"

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    TC_FFT fft_output[N])
{
    static fft<N, n_clog2_c> fft_obj;
    static input_reorder_buffer<N, n_clog2_c> input_reorder_buffer_obj;

    TC_FFT fft_input_lower[N / 2];
    TC_FFT fft_input_upper[N / 2];
    TC_FFT fft_output_lower[N / 2];
    TC_FFT fft_output_upper[N / 2];

    // Store samples in buffer
    input_reorder_buffer_obj.store_sample(
        // Inputs
        input_signal,
        // Outputs
        fft_input_lower,
        fft_input_upper);

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
