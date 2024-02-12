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
    TC_FFT fft_input_lower[n_instances_c][N / 2];
    TC_FFT fft_input_upper[n_instances_c][N / 2];
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
    }

    void store_sample(
        // Inputs
        TI_INPUT_SIGNAL sample_in)
    {
        idx_reordered = TUI_SAMPLE_ARRAY_IDX(sample_count % N).reverse();
        if (idx_reordered[0] == 0)
        {
            fft_input_lower[sample_count / N][idx_reordered >> 1] = TC_FFT(TR_FFT(sample_in), 0);
            fprintf(stdout, "idx_lower = %d, sample_num = %d\n", idx_reordered >> 1, sample_count);
        }
        else
        {
            fft_input_upper[sample_count / N][N / 2 - 1 - (idx_reordered >> 1)] = TC_FFT(TR_FFT(sample_in), 0);
            fprintf(stdout, "idx_upper = %d, sample_num = %d\n", N / 2 - 1 - (idx_reordered >> 1), sample_count);
        }

        if (sample_count % N == N - 1)
        {
            buffer_full[sample_count / N] = 1;
        }

        sample_count++;
    }
};