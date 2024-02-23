#include "input_reorder_buffer_wrapper.h"

void input_reorder_buffer_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    TC_FFT fft_input_lower[N / 2],
    TC_FFT fft_input_upper[N / 2],
    TB &buffer_full)
{
    TI_INPUT_SIGNAL sample_in;
    static input_reorder_buffer<N, n_clog2_c> input_reorder_buffer_obj;

    // Store sample in buffer
    input_signal.read(sample_in);
    input_reorder_buffer_obj.store_sample(
        // Inputs
        sample_in,
        // Outputs
        fft_input_lower,
        fft_input_upper,
        buffer_full);
}