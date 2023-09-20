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
        sample_cnt = 0;
        m = 0;

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
        T_C c;
        // Compute intermediate values for better readability and efficiency
        typename T_C::value_type term1 = a.real() * (b.real() - b.imag());
        typename T_C::value_type term2 = b.imag() * (a.real() - a.imag());
        typename T_C::value_type term3 = a.imag() * (b.real() + b.imag());

        // Complex multiplication formula with three multiplicative operations
        c.real(term1 + term2);
        c.imag(term3 + term2);

        return c;
    }

    void computeFFTMagnitude(
        // Inputs
        hls::stream<TI_INPUT_SIGNAL> &input_signal,
        TB &start,
        // Outputs
        TC_FFT fft_output[N],
        TB &done)
    {
        done = 1;
        if (start)
        {
            done = 0;

            if (input_signal.read_nb(input_signal_tmp))
            {
            SHIFT_REGISTER:
                for (int n = 0; n < N - 1; n++)
                {
                    sample_array[n] = sample_array[n + 1]; // Shift elements one position to the left
                }
                sample_array[N - 1] = input_signal_tmp;
                sample_cnt++;
            }

            if (sample_cnt == N)
            {
            INIT_FFT_RESULTS_ARRAY:
                for (int n = 0; n < N; n++)
                {
                    fft_result[n] = sample_array[bit_reversed_idx[n]];
                }

                for (int s = 1; s <= n_clog2_c; s++)
                {
                    for (int k = 0; k < N; k++)
                    {
                        if (k == 0)
                        {
                            m = 1 << s;
                        }
                        if (k % m == 0)
                        {
                            TC_TWIDDLE_FACTOR w = TC_TWIDDLE_FACTOR(1.0, 0.0);
                            for (int j = 0; j < m / 2; j++)
                            {
                                TC_FFT t = comp_mult_three_dsp<TC_TWIDDLE_FACTOR, TC_FFT, TC_FFT>(w, fft_result[k + j + m / 2]);
                                TC_FFT u = fft_result[k + j];
                                fft_result[k + j] = u + t;
                                fft_result[k + j + m / 2] = u - t;
                                w = comp_mult_three_dsp<TC_TWIDDLE_FACTOR, TC_TWIDDLE_FACTOR, TC_TWIDDLE_FACTOR>(w, twiddle_factors[N / m]);
                            }
                        }
                    }
                }

            FFT_MAGNITUDE_CALC:
                for (int k = 0; k < N; k++)
                {
                    fft_output[k] = fft_result[k];
                }
                start = 0;
                done = 1;
                sample_cnt = 0;
            }
        }
    }

private:
    TC_TWIDDLE_FACTOR twiddle_factors[N]; // Pre-computed twiddle factors
    TI_INPUT_SIGNAL sample_array[N];
    TUI_SAMPLE_ARRAY_IDX bit_reversed_idx[N];
    TC_FFT fft_result_aux;
    TC_FFT fft_result[N];
    TI_INPUT_SIGNAL input_signal_tmp;
    TB start;
    ap_uint<n_clog2_c + 1> sample_cnt;
    ap_uint<n_clog2_c + 1> m;
};
