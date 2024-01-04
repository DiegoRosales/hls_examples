#include "fft_wrapper.h"

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    TC_FFT fft_output[N],
    TB &valid)
{
    static fft<N, n_clog2_c> fft_obj;
    static input_reorder_buffer<N, n_clog2_c, 1> input_reorder_buffer_obj;
    TI_INPUT_SIGNAL sample_in;

    // Store sample in buffer
    if (input_signal.read_nb(sample_in))
    {
        input_reorder_buffer_obj.store_sample(sample_in);
    }

    if (input_reorder_buffer_obj.buffer_full[0])
    {
        input_reorder_buffer_obj.buffer_full[0] = 0;

        fft_obj.doFFT(input_reorder_buffer_obj.fft_input[0], fft_output);

        valid = fft_obj.done;
    }
}
