#include "fft_wrapper.h"

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    TC_FFT fft_output[N])
{
    static fft<N, n_clog2_c> fft_obj;
    static input_reorder_buffer<N, n_clog2_c> input_reorder_buffer_obj;
    TI_INPUT_SIGNAL sample_in;
    TC_FFT fft_output_lower[N / 2];
    TC_FFT fft_output_upper[N / 2];

    // Store sample in buffer
    if (input_signal.read_nb(sample_in))
    {
        input_reorder_buffer_obj.store_sample(sample_in);
    }

    if (input_reorder_buffer_obj.buffer_full)
    {
        input_reorder_buffer_obj.buffer_full = 0;
        fft_obj.doFFT(input_reorder_buffer_obj.fft_input_lower, input_reorder_buffer_obj.fft_input_upper, fft_output_lower, fft_output_upper);

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
}
