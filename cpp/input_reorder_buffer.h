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
    TC_FFT fft_input[N];

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
        TB &buffer_full)
    {
        idx_reordered = sample_count.range(0, n_clog2_c - 1);
        fft_input[idx_reordered] = TC_FFT(TR_FFT(sample_in), 0);

        buffer_full = (sample_count == N - 1) ? 1 : 0;

        sample_count++;
    }
};