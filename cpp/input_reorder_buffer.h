#include <complex>
#include "fft_sysdef.h"
#include "hls_stream.h"

template <int N, int n_clog2_c, int n_instances_c>
class input_reorder_buffer
{
private:
    TUI_SAMPLE_ARRAY_IDX sample_count;
    TUI_SAMPLE_ARRAY_IDX idx_reordered;

public:
    TC_FFT fft_input[n_instances_c][N];
    TB buffer_full[n_instances_c];

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
        for (int i = 0; i < n_instances_c; i++)
        {
            buffer_full[i] = 0;
        }
    }

    void store_sample(
        // Inputs
        TI_INPUT_SIGNAL sample_in)
    {
        idx_reordered = TUI_SAMPLE_ARRAY_IDX(sample_count % N).range(0, n_clog2_c - 1);
        fft_input[sample_count / N][idx_reordered] = TC_FFT(sample_in, 0);
        if (sample_count % N == N - 1)
        {
            buffer_full[sample_count / N] = 1;
        }
        else
        {
            sample_count++;
        }
    }
};