#include "hls_math.h"
#include <complex>
#include "fft_sysdef.h"
#include "hls_stream.h"

template <int N, int n_clog2_c>
class fft
{
public:
    TC_TWIDDLE_FACTOR twiddle_factors[N / 2]; // Pre-computed twiddle factors
    TC_FFT fft_stage_input[N];
    TC_FFT fft_stage_output[N];
    ap_uint<n_clog2_c + 1> m[n_clog2_c];
    TB valid;

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

    // Reset function
    void reset()
    {
        valid = 0;

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
    }

    template <class T_A, class T_B, class T_C>
    T_C comp_mult_three_dsp(const T_A &a, const T_B &b)
    {
        return T_C((a.real() * (b.real() - b.imag()) + b.imag() * (a.real() - a.imag())),
                   (a.imag() * (b.real() + b.imag()) + b.imag() * (a.real() - a.imag())));
    }

    template <class T_IN, class T_OUT>
    void ButterflyOperator(T_IN data_in[N], T_OUT data_out[N], int fft_stage_num)
    {
        int j;
        int k;
        TC_FFT product;
    BUTTERFLY_MULTIPLICATION:
        for (int i = 0; i < N / 2; i++)
        {
#pragma HLS dependence variable = data_in type = inter false
#pragma HLS dependence variable = data_in type = intra false
#pragma HLS dependence variable = data_out type = inter false
#pragma HLS dependence variable = data_out type = intra false
            j = i % (m[fft_stage_num] / 2);
            if (j == 0)
            {
                k = i * 2;
            }
            product = comp_mult_three_dsp<TC_TWIDDLE_FACTOR, TC_FFT, TC_FFT>(twiddle_factors[j * N / m[fft_stage_num]], data_in[k + j + m[fft_stage_num] / 2]);
            data_out[k + j] = data_in[k + j] + product;
            data_out[k + j + m[fft_stage_num] / 2] = data_in[k + j] - product;
        }
    }

    void doFFT(
        // Inputs
        TC_FFT fft_input[N],
        // Outputs
        TC_FFT fft_output[N])
    {
        // Calculate first stage
        ButterflyOperator<TC_FFT, TC_FFT>(fft_input, fft_stage_output, 0);

    BUTTERFLY_OPERATOR_LOOP:
        for (int s = 1; s < n_clog2_c; s++)
        {
            if (s % 2 == 0)
            {
                ButterflyOperator<TC_FFT, TC_FFT>(fft_stage_input, fft_stage_output, s);
            }
            else
            {
                ButterflyOperator<TC_FFT, TC_FFT>(fft_stage_output, fft_stage_input, s);
            }
        }

    OUTPUT_MAPPING_LOOP:
        for (int k = 0; k < N; k++)
        {
            valid = 0;
            if (n_clog2_c % 2 == 0)
            {
                fft_output[k] = fft_stage_input[k];
            }
            else
            {
                fft_output[k] = fft_stage_output[k];
            }
            valid = 1;
        }
    }
};
