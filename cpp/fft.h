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

    // Initialize the twiddle factors and arrays
    INIT_TWIDDLE_FACTORS:
        for (int k = 0; k < N; k++)
        {
            // fprintf(stdout, "twiddle_factor_double[%d] = %lf + i%lf\n", k, twiddle_factor_k.real(), twiddle_factor_k.imag());
            twiddle_factors[k] = computeTwiddleFactor(k);
            // fprintf(stdout, "twiddle_factors_fixed[%d] = %lf + i%lf\n", k, twiddle_factors[k].real().to_double(), twiddle_factors[k].imag().to_double());
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
            // fprintf(stdout, "bit_reversed_idx[%d] = %d\n", n, bit_reversed_idx[n].to_int());
        }
    }

    template <class T_A, class T_B, class T_C>
    T_C comp_mult_three_dsp(const T_A &a, const T_B &b)
    {
        T_C c;
        c.real(b.real() + a.real() * b.imag() - (a.real() + a.imag()) * b.imag());
        c.imag(b.real() + a.real() * b.imag() + (a.imag() - a.real()) * b.real());
        return c;
    }

    void computeFFTMagnitude(
        // Inputs
        hls::stream<TR_INPUT_SIGNAL> &input_signal,
        TB &start,
        // Outputs
        TR_FFT_NORM fft_magnitudes[N],
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
                // fprintf(stdout, "sample_cnt = %d\n", sample_cnt.to_int());
            }

            if (sample_cnt == 1024)
            {
                fprintf(stdout, "-------------STARTING CALCULATION-------------\n");
            INIT_FFT_RESULTS_ARRAY:
                for (int n = 0; n < N; n++)
                {
                    fft_result[n] = sample_array[bit_reversed_idx[n]];
                }
                fprintf(stdout, "-------------BIT REVERSED-------------\n");

                // Butterfly computation
                for (int s = 1; s <= n_clog2_c; s++)
                {
                    ap_uint<n_clog2_c + 1> m = 1 << s;
                    for (int k = 0; k < N; k += m)
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
                fprintf(stdout, "-------------BUTTERFLY CALCULATION PERFORMED-------------\n");

            FFT_MAGNITUDE_CALC:
                for (int k = 0; k < N / 2; k++)
                {
                    fft_magnitudes[k] = TR_FFT_NORM(fft_result[k].real() * fft_result[k].real() + fft_result[k].imag() * fft_result[k].imag());
                }
                fprintf(stdout, "-------------END-------------\n");
                // fprintf(stdout, "done = %d\n", done);
                start = 0;
                done = 1;
                sample_cnt = 0;
                // fprintf(stdout, "done = %d\n", done);
            }
        }
    }

private:
    TC_TWIDDLE_FACTOR twiddle_factors[N]; // Pre-computed twiddle factors
    TR_INPUT_SIGNAL sample_array[N];
    TUI_SAMPLE_ARRAY_IDX bit_reversed_idx[N];
    TC_FFT fft_result_aux;
    TC_FFT fft_result[N / 2];
    TR_INPUT_SIGNAL input_signal_tmp;
    TB start;
    ap_uint<n_clog2_c + 1> sample_cnt;
};
