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
        double pi = 3.14159265358979323846;
        double angle = -2.0 * pi * k / N;
        TR_TWIDDLE_FACTOR realPart = TR_TWIDDLE_FACTOR(cos(angle));
        TR_TWIDDLE_FACTOR imagPart = TR_TWIDDLE_FACTOR(sin(angle));
        return TC_TWIDDLE_FACTOR(realPart, imagPart);
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
        for (int k = 0; k < N; k++)
        {
            twiddle_factors[k] = computeTwiddleFactor(k);
        }
    INIT_SAMPLE_ARRAY:
        for (int n = 0; n < N; n++)
        {
            sample_array[n] = 0;
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

    void computeFFTMagnitude(
        // Inputs
        hls::stream<TI_INPUT_SIGNAL> &input_signal,
        // Outputs
        TC_FFT fft_output[N])
    {
#pragma HLS ARRAY_PARTITION variable = twiddle_factors type = complete dim = 1
#pragma HLS ARRAY_PARTITION variable = sample_array type = complete dim = 1
#pragma HLS ARRAY_PARTITION variable = bit_reversed_idx type = complete dim = 1
#pragma HLS ARRAY_PARTITION variable = fft_result type = complete dim = 1
#pragma HLS ARRAY_PARTITION variable = m type = complete dim = 1
        if (input_signal.read_nb(input_signal_tmp))
        {
        SHIFT_REGISTER:
            for (int n = 0; n < N - 1; n++)
            {
                sample_array[n] = sample_array[n + 1]; // Shift elements one position to the left
            }
            sample_array[N - 1] = input_signal_tmp;
        }

    INIT_FFT_RESULTS_ARRAY:
        for (int n = 0; n < N; n++)
        {
            fft_result[n] = sample_array[bit_reversed_idx[n]];
        }

    BUTTERFLY_MULTIPLICATION_S:
        for (int s = 0; s < n_clog2_c; s++)
        {
        BUTTERFLY_MULTIPLICATION_I:
            for (int i = 0; i < N / m[s]; i++)
            {
                k = i * m[s];
                w = TC_TWIDDLE_FACTOR(1.0, 0.0);
            BUTTERFLY_MULTIPLICATION_J:
                for (int j = 0; j < m[s] / 2; j++)
                {
                    t = comp_mult_three_dsp<TC_TWIDDLE_FACTOR, TC_FFT, TC_FFT>(w, fft_result[k + j + m[s] / 2]);
                    u = fft_result[k + j];
                    fft_result[k + j] = u + t;
                    fft_result[k + j + m[s] / 2] = u - t;
                    w = comp_mult_three_dsp<TC_TWIDDLE_FACTOR, TC_TWIDDLE_FACTOR, TC_TWIDDLE_FACTOR>(w, twiddle_factors[N / m[s]]);
                }
            }
        }

    FFT_MAGNITUDE_CALC:
        for (int k = 0; k < N; k++)
        {
            fft_output[k] = fft_result[k];
        }
    }

private:
    TC_TWIDDLE_FACTOR twiddle_factors[N]; // Pre-computed twiddle factors
    TC_TWIDDLE_FACTOR w;
    TI_INPUT_SIGNAL sample_array[N];
    TUI_SAMPLE_ARRAY_IDX bit_reversed_idx[N];
    TC_FFT fft_result_aux;
    TC_FFT fft_result[N];
    TC_FFT t, u;
    TI_INPUT_SIGNAL input_signal_tmp;
    ap_uint<n_clog2_c + 1> m[n_clog2_c];
    ap_uint<n_clog2_c> k;
};
