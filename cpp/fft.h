#pragma once
#include "hls_math.h"
#include <complex>
#include "fft_sysdef.h"
#include "hls_stream.h"

template <int N, int n_clog2_c>
class fft
{
public:
    TC_TWIDDLE_FACTOR twiddle_factors[N / 2]; // Pre-computed twiddle factors
    TC_FFT fft_stage_lower[n_clog2_c - 1][N / 2];
    TC_FFT fft_stage_upper[n_clog2_c - 1][N / 2];

    // Indexes arrays
    TUI_SAMPLE_ARRAY_IDX idx_upper[n_clog2_c][N / 2];
    TUI_SAMPLE_ARRAY_IDX twiddle_factor_idx[n_clog2_c][N / 2];

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

    INIT_INDEXES:
        for (int s = 0; s < n_clog2_c; s++)
        {
            int m = (N / 2) >> s;
            for (int i = 0; i < N / 2; i++)
            {
                twiddle_factor_idx[s][i] = TUI_SAMPLE_ARRAY_IDX((i * m) % (N / 2));
                idx_upper[s][i] = i ^ (-1 << s) + N / 2;
            }
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
    void ButterflyOperator(
        // Inputs
        T_IN data_in_lower[N / 2],
        T_IN data_in_upper[N / 2],
        TUI_SAMPLE_ARRAY_IDX twiddle_factor_idx[N / 2],
        TUI_SAMPLE_ARRAY_IDX idx_upper[N / 2],
        int s,
        // Outputs
        T_OUT data_out_lower[N / 2],
        T_OUT data_out_upper[N / 2])
    {
#pragma HLS INLINE OFF
#pragma HLS FUNCTION_INSTANTIATE variable = s
    BUTTERFLY_MULTIPLICATION:
        for (int i = 0; i < N / 2; i++)
        {
#pragma HLS pipeline II = 1
            T_IN data_lower_swapped;
            T_IN data_upper_swapped;
            TUI_SAMPLE_ARRAY_IDX idx = TUI_SAMPLE_ARRAY_IDX(i);
            TB swap_previous_stage = s != 0 && (idx & (1 << (s - 1)));
            TB swap_post_stage = idx.test(s);
            if (swap_previous_stage)
            {
                // Swap input data
                data_lower_swapped = data_in_upper[idx_upper[i]];
                data_upper_swapped = data_in_lower[i];
            }
            else
            {
                data_lower_swapped = data_in_lower[i];
                data_upper_swapped = data_in_upper[idx_upper[i]];
            }

            TC_FFT product = comp_mult_three_dsp<TC_TWIDDLE_FACTOR, TC_FFT, TC_FFT>(twiddle_factors[twiddle_factor_idx[i]], data_upper_swapped);

            // Swap output data
            if (swap_post_stage)
            {
                data_out_lower[i] = data_lower_swapped - product;
                data_out_upper[idx_upper[i]] = data_lower_swapped + product;
            }
            else
            {
                data_out_lower[i] = data_lower_swapped + product;
                data_out_upper[idx_upper[i]] = data_lower_swapped - product;
            }
        }
    }

    void doFFT(
        // Inputs
        TC_FFT fft_input_lower[N / 2],
        TC_FFT fft_input_upper[N / 2],
        // Outputs
        TC_FFT fft_output_lower[N / 2],
        TC_FFT fft_output_upper[N / 2])
    {
#pragma HLS dataflow
        // Calculate first stage
        ButterflyOperator<TC_FFT, TC_FFT>(fft_input_lower, fft_input_upper, twiddle_factor_idx[0], idx_upper[0], 0, fft_stage_lower[0], fft_stage_upper[0]);

    BUTTERFLY_OPERATOR_LOOP:
        for (int s = 1; s < n_clog2_c - 1; s++)
        {
#pragma HLS unroll
            ButterflyOperator<TC_FFT, TC_FFT>(fft_stage_lower[s - 1], fft_stage_upper[s - 1], twiddle_factor_idx[s], idx_upper[s], s, fft_stage_lower[s], fft_stage_upper[s]);
        }

        ButterflyOperator<TC_FFT, TC_FFT>(fft_stage_lower[n_clog2_c - 2], fft_stage_upper[n_clog2_c - 2], twiddle_factor_idx[n_clog2_c - 1], idx_upper[n_clog2_c - 1], n_clog2_c - 1, fft_output_lower, fft_output_upper);
    }
};
