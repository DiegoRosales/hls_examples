#pragma once
#include <complex>
#include "fft_sysdef.h"
#include "hls_stream.h"

template <int N, int n_clog2_c>
class input_reorder_buffer
{
private:
    TUI_SAMPLE_ARRAY_COUNT sample_count;

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
        hls::stream<TI_INPUT_SIGNAL> &input_signal,
        // Outputs
        TC_FFT fft_input_lower[N / 2],
        TC_FFT fft_input_upper[N / 2])
    {
        TI_INPUT_SIGNAL sample_in;
        TUI_SAMPLE_ARRAY_IDX idx_reordered;

        while (sample_count < N)
        {
            input_signal.read(sample_in);
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
            sample_count++;
        }
        sample_count = 0;
    }
};