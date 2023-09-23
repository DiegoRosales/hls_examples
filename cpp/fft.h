#include "hls_math.h"
#include <complex>
#include "fft_sysdef.h"
#include "hls_stream.h"

template <int N, int n_clog2_c>
class fft
{
public:
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

    template <class T>
    void ArrayReorder(T data_in[N], T data_out[N])
    {
    ARRAY_REORDER_LOOP:
        for (int n = 0; n < N; n++)
        {
            data_out[n] = data_in[bit_reversed_idx[n]];
        }
    }

    template <class T_IN, class T_OUT>
    void ButterflyOperator(T_IN data_in[N], T_OUT data_out[N], int fft_stage_num)
    {
        int k;
    BUTTERFLY_MULTIPLICATION_I:
        for (int i = 0; i < N / m[fft_stage_num]; i++)
        {
            k = i * m[fft_stage_num];
        BUTTERFLY_MULTIPLICATION_J:
            for (int j = 0; j < m[fft_stage_num] / 2; j++)
            {
                product = comp_mult_three_dsp<TC_TWIDDLE_FACTOR, TC_FFT, TC_FFT>(twiddle_factors[j * N / m[fft_stage_num]], data_in[k + j + m[fft_stage_num] / 2]);
                data_out[k + j] = data_in[k + j] + product;
                data_out[k + j + m[fft_stage_num] / 2] = data_in[k + j] - product;
            }
        }
    }

    void computeFFTMagnitude(
        // Inputs
        TI_INPUT_SIGNAL input_signal[N],
        // Outputs
        TC_FFT fft_output[N])
    {

        ArrayReorder<TI_INPUT_SIGNAL>(input_signal, sample_array_reordered);

        for (int n = 0; n < N; n++)
        {
            fft_stages[0][n] = TC_FFT(sample_array_reordered[n], 0);
        }

        for (int s = 0; s < n_clog2_c; s++)
        {
            ButterflyOperator<TC_FFT, TC_FFT>(fft_stages[s], fft_stages[s + 1], s);
        }

    OUTPUT_MAPPING_LOOP:
        for (int k = 0; k < N; k++)
        {
            fft_output[k] = fft_stages[10][k];
        }
    }

private:
    TC_TWIDDLE_FACTOR twiddle_factors[N / 2]; // Pre-computed twiddle factors
    TI_INPUT_SIGNAL sample_array_reordered[N];
    TUI_SAMPLE_ARRAY_IDX bit_reversed_idx[N];
    TC_FFT fft_stages[n_clog2_c + 1][N];
    TC_FFT product;
    ap_uint<n_clog2_c + 1> m[n_clog2_c];
};
