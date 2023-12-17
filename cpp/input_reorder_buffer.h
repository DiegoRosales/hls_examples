#include <complex>
#include "fft_sysdef.h"
#include "hls_stream.h"

template <int N, int n_clog2_c>
class input_reorder_buffer
{
private:
    TUI_SAMPLE_ARRAY_IDX sample_count;
    TI_INPUT_SIGNAL sample_in;

public:
    TC_FFT fft_input[N];
    TB buffer_full;

    // Constructor
    input_reorder_buffer(void)
    {
        reset();
    }

    // Destructor
    ~input_reorder_buffer(void) {}

    void reset()
    {
        buffer_full = 0;
        sample_count = 0;
    }

    void store_sample(
        // Inputs
        hls::stream<TI_INPUT_SIGNAL> &input_signal)
    {
        if (input_signal.read_nb(sample_in))
        {
            fft_input[sample_count.range(0, n_clog2_c - 1)] = TC_FFT(sample_in, 0);
            if (sample_count == N - 1)
            {
                buffer_full = 1;
            }
            else
            {
                sample_count++;
            }
        }
    }
};