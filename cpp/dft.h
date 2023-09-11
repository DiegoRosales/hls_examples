#include "hls_math.h"
#include <complex>
#include "dft_sysdef.h"
#include "hls_stream.h"

template <int N, int n_clog2_c>
class dft
{
public:
    // Constructor
    dft(void)
    {
        reset();
    }

    // Destructor
    ~dft(void) {}

    TC_TWIDDLE_FACTOR twiddle_factors[N]; // Pre-computed twiddle factors

    // Compute twiddle factors at compile time
    TC_TWIDDLE_FACTOR computeTwiddleFactor(int k)
    {
        double pi = 3.14159265358979323846;
        double angle = -2.0 * pi * k / N;
        TR_TWIDDLE_FACTOR realPart = TR_TWIDDLE_FACTOR(cos(angle));
        TR_TWIDDLE_FACTOR imagPart = TR_TWIDDLE_FACTOR(sin(angle));
        return TC_TWIDDLE_FACTOR(realPart, imagPart);
    }

    // Reset function
    void reset()
    {
        sample_cnt = 0;
        start = 0;

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
    }

    void computeDFTMagnitude(
        // Inputs
        hls::stream<TR_INPUT_SIGNAL> &input_signal,
        // Outputs
        TR_DFT_NORM dft_magnitudes[N / 2])
    {
        if (input_signal.read_nb(input_signal_tmp))
        {
        SHIFT_REGISTER:
            for (int n = 0; n < N - 1; n++)
            {
                sample_array[n] = sample_array[n + 1];
                // fprintf(stdout, "sample_array[%d] = %lf\n", n, sample_array[n].to_double());
            }
            sample_array[N - 1] = input_signal_tmp;
            // fprintf(stdout, "sample_array[%d] = %lf\n", N - 1, sample_array[N - 1].to_double());
            // fprintf(stdout, "input_signal_tmp = %lf\n", input_signal_tmp.to_double());
            sample_cnt++;
            // fprintf(stdout, "sample_cnt = %d\n", sample_cnt.to_int());
            if (sample_cnt == 1023)
            {
                start = 1;
            }

            if (start)
            {
            RESET_RESULTS_ARRAY:
                for (int k = 0; k < N / 2; k++)
                {
                    dft_result[k] = 0;
                }

            DFT_CALC_K:
                for (int k = 0; k < N / 2; k++)
                {
                DFT_CALC_N:
                    for (int n = 0; n < N; n++)
                    {
                        // fprintf(stdout, "twiddle_factors_fixed[%d] = %lf + i%lf\n", (k * n) % N, twiddle_factors[(k * n) % N].real().to_double(), twiddle_factors[(k * n) % N].imag().to_double());
                        dft_result_aux = TC_DFT(sample_array[n] * twiddle_factors[(k * n) % N].real(),
                                                sample_array[n] * twiddle_factors[(k * n) % N].imag());
                        // fprintf(stdout, "sample_array[%d] = %lf\n", n, sample_array[n].to_double());
                    }
                    dft_result[k] += dft_result_aux;
                }
            DFT_MAGNITUDE_CALC:
                for (int k = 0; k < N / 2; k++)
                {
                    // fprintf(stdout, "dft_result[%d] = %lf + i%lf\n", k, dft_result[k].real().to_double(), dft_result[k].imag().to_double());
                    dft_magnitudes[k] = TR_DFT_NORM((dft_result[k].real() * dft_result[k].real() + dft_result[k].imag() * dft_result[k].imag()));
                    // fprintf(stdout, "dft_magnitudes[%d] = %lf\n", k, dft_magnitudes[k].to_double());
                }
            }
        }
    }

private:
    TR_INPUT_SIGNAL sample_array[N];
    ap_uint<n_clog2_c> sample_cnt;
    TC_DFT dft_result_aux;
    TC_DFT dft_result[N / 2];
    TR_INPUT_SIGNAL input_signal_tmp;
    TB start;
};
