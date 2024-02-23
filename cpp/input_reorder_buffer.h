#pragma once
#include <complex>
#include "fft_sysdef.h"
#include "hls_stream.h"

template <int N, int n_clog2_c>
class input_reorder_buffer
{
private:
    TUI_SAMPLE_ARRAY_IDX sample_count;
    TUI_SAMPLE_ARRAY_IDX idx_reordered;

public:
    // Constructor
    input_reorder_buffer(void)
    {
        reset();
    }

    // Destructor
    ~input_reorder_buffer(void) {}

    void reset()
    {
        sample_count = 0;
    }

    void store_sample(
        // Inputs
        TI_INPUT_SIGNAL sample_in,
        // Outputs
        TC_FFT fft_input_lower[N / 2],
        TC_FFT fft_input_upper[N / 2],
        TB &buffer_full)
    {
        idx_reordered = sample_count;
        idx_reordered.reverse();
        if (idx_reordered.test(0))
        {
            fft_input_upper[N / 2 - 1 - (idx_reordered >> 1)] = TC_FFT(TR_FFT(sample_in), 0);
        }
        else
        {
            fft_input_lower[idx_reordered >> 1] = TC_FFT(TR_FFT(sample_in), 0);
        }

        if (sample_count == N - 1)
        {
            buffer_full = 1;
        }

        sample_count++;
    }
};