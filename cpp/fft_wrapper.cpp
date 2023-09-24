#include "fft_wrapper.h"

TUI_SAMPLE_ARRAY_IDX sample_count = 0;
TB buffer_full = 0;
TI_INPUT_SIGNAL input_reordered[N];

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    TC_FFT fft_output[N])
{
    static fft<N, n_clog2_c> fft_obj;
    if (!buffer_full)
    {
        input_signal.read(input_reordered[fft_obj.bit_reversed_idx[sample_count]]);
        if (sample_count == N - 1)
        {
            buffer_full = 1;
        }
        else
        {
            sample_count++;
        }
    }
    else
    {
        fft_obj.doFFT(input_reordered, fft_output);
        buffer_full = 0;
    }
}
