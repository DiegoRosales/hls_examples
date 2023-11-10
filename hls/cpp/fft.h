#include "hls_math.h"
#include <complex>
#include "fft_sysdef.h"
#include "hls_stream.h"

template <int N, int n_clog2_c>
class fft
{
public:
    TC_TWIDDLE_FACTOR twiddle_factors[N / 2]; // Pre-computed twiddle factors

    TUI_SAMPLE_ARRAY_IDX bit_reversed_idx[N];
    TC_FFT fft_stage_input[N];
    TC_FFT fft_stage_output[N];
    ap_uint<n_clog2_c + 1> m[n_clog2_c];

    // Constructor
    fft(void)
    {
        reset();
    }

    // Destructor
    ~fft(void) {}

    // Compute twiddle factors at compile time
    TC_TWIDDLE_FACTOR computeTwiddleFactor(int k)
    {
        double angle = -2.0 * M_PI * k / N;
        return TC_TWIDDLE_FACTOR(cos(angle), sin(angle));
    }

    TUI_SAMPLE_ARRAY_IDX reverse_bits(TUI_SAMPLE_ARRAY_IDX index)
    {
        return index.range(0, n_clog2_c - 1);
    }

    // Reset function
    void reset()
    {

    INIT_M_ARRAY:
        for (int i = 0; i < n_clog2_c; i++)
        {
            m[i] = 1 << (i + 1);
        }

        // Initialize the twiddle factors and arrays
    INIT_TWIDDLE_FACTORS:
        for (int k = 0; k < N / 2; k++)
        {
            twiddle_factors[k] = computeTwiddleFactor(k);
        }
    INIT_BIT_REVERSED_INDEXES:
        for (int n = 0; n < N; n++)
        {
            bit_reversed_idx[n] = reverse_bits(TUI_SAMPLE_ARRAY_IDX(n));
        }
    }

    template <class T_A, class T_B, class T_C>
    T_C comp_mult_three_dsp(const T_A &a, const T_B &b)
    {
        return T_C((a.real() * (b.real() - b.imag()) + b.imag() * (a.real() - a.imag())),
                   (a.imag() * (b.real() + b.imag()) + b.imag() * (a.real() - a.imag())));
    }

    template <class T_IN, class T_OUT>
    void ButterflyOperator(T_IN data_in[N], T_OUT data_out[N], ap_uint<n_clog2_c + 1> curr_m)
    {
        #pragma HLS inline
        int j;
        int k;
        TC_FFT product;
    BUTTERFLY_MULTIPLICATION:
        for (int i = 0; i < N / 2; i++)
        {
            j = i % (curr_m / 2);
            if (j == 0)
            {
                k = i * 2;
            }
            product = comp_mult_three_dsp<TC_TWIDDLE_FACTOR, TC_FFT, TC_FFT>(twiddle_factors[j * N / curr_m], data_in[k + j + curr_m / 2]);
            data_out[k + j] = data_in[k + j] + product;
            data_out[k + j + curr_m / 2] = data_in[k + j] - product;
        }
    }

    void doFFT(
        // Inputs
        TI_INPUT_SIGNAL input_reordered[N],
        // Outputs
        TC_FFT fft_output[N])
    {
        
    INIT_FIRST_STAGE:
        for (int n = 0; n < N; n++)
        {
            fft_stage_output[n] = TC_FFT(input_reordered[n], 0);
        }

    BUTTERFLY_OPERATOR_LOOP:
        for (int s = 0; s < n_clog2_c; s++)
        {
        BUTTERFLY_OPERATOR_ASSIGN_INPUT:
            for (int i = 0; i < N; i++)
            {
                fft_stage_input[i] = fft_stage_output[i];
            }
            ButterflyOperator<TC_FFT, TC_FFT>(fft_stage_input, fft_stage_output, m[s]);
        }

    OUTPUT_MAPPING_LOOP:
        for (int k = 0; k < N; k++)
        {
            fft_output[k] = fft_stage_output[k];
        }
    }
};
