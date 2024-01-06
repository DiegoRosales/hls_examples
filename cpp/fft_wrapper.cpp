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
    TB buffer_full;

    // Store sample in buffer
    if (input_signal.read_nb(sample_in))
    {
        input_reorder_buffer_obj.store_sample(sample_in, buffer_full);
    }

    if (buffer_full)
    {
        fft_obj.doFFT(input_reorder_buffer_obj.fft_input, fft_output);
    }
}
