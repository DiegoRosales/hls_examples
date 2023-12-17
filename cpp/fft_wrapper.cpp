#include "fft_wrapper.h"

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    TC_FFT fft_output[N],
    TB &valid)
{
    static fft<N, n_clog2_c> fft_obj;
    static input_reorder_buffer<N, n_clog2_c> input_reorder_buffer_obj;

    // Store sample in buffer
    input_reorder_buffer_obj.store_sample(input_signal);

    if (input_reorder_buffer_obj.buffer_full)
    {
        input_reorder_buffer_obj.buffer_full = 0;
        fft_obj.doFFT(input_reorder_buffer_obj.fft_input, fft_output);
        valid = fft_obj.valid;
    }
}
