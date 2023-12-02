#include "fft_wrapper.h"

TUI_SAMPLE_ARRAY_IDX sample_count = 0;
TB buffer_full = 0;

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    TC_FFT fft_output[N])
{
    TI_INPUT_SIGNAL sample_in;
    TC_FFT fft_input[N];
    static fft<N, n_clog2_c> fft_obj;
    if (input_signal.read_nb(sample_in))
    {
        fft_input[fft_obj.bit_reversed_idx[sample_count]] = TC_FFT(sample_in, 0);
        if (sample_count == N - 1)
        {
            buffer_full = 1;
        }
        else
        {
            sample_count++;
        }
    }
    if (buffer_full)
    {
        buffer_full = 0;
        fft_obj.doFFT(fft_input, fft_output);
    }
}
