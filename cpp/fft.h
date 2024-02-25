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

    // Structure to hold pre-computed indexes
    // Merging these arrays inside a struct allows Vitis HLS to use a single ROM to store the values
    struct T_PRECOMPUTED_INDEXES
    {
        TUI_SAMPLE_ARRAY_IDX idx_upper;
        TUI_SAMPLE_ARRAY_IDX twiddle_factor_idx;
    };
    T_PRECOMPUTED_INDEXES precomputed_idx[n_clog2_c][N / 2];

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

        // Initialize pre-computed indexes
    INIT_INDEXES:
        for (int stage_num = 0; stage_num < n_clog2_c; stage_num++)
        {
            int m = (N / 2) >> stage_num;
            for (int i = 0; i < N / 2; i++)
            {
                precomputed_idx[stage_num][i].twiddle_factor_idx = TUI_SAMPLE_ARRAY_IDX((i * m) % (N / 2));
                precomputed_idx[stage_num][i].idx_upper = i ^ (-1 << stage_num) + N / 2;
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
        T_PRECOMPUTED_INDEXES precomputed_idx[N / 2],
        int stage_num,
        // Outputs
        T_OUT data_out_lower[N / 2],
        T_OUT data_out_upper[N / 2])
    {
#pragma HLS INLINE OFF
#pragma HLS FUNCTION_INSTANTIATE variable = stage_num
    BUTTERFLY_MULTIPLICATION:
        for (int i = 0; i < N / 2; i++)
        {
#pragma HLS pipeline II = 1
            T_IN data_lower_swapped;
            T_IN data_upper_swapped;
            TUI_SAMPLE_ARRAY_IDX idx = TUI_SAMPLE_ARRAY_IDX(i);
            TB swap_previous_stage = stage_num != 0 && idx.test(stage_num - 1);
            TB swap_post_stage = idx.test(stage_num);
            if (swap_previous_stage)
            {
                // Swap input data
                data_lower_swapped = data_in_upper[precomputed_idx[i].idx_upper];
                data_upper_swapped = data_in_lower[i];
            }
            else
            {
                data_lower_swapped = data_in_lower[i];
                data_upper_swapped = data_in_upper[precomputed_idx[i].idx_upper];
            }

            TC_FFT product = comp_mult_three_dsp<TC_TWIDDLE_FACTOR, TC_FFT, TC_FFT>(twiddle_factors[precomputed_idx[i].twiddle_factor_idx], data_upper_swapped);

            // Swap output data
            if (swap_post_stage)
            {
                data_out_lower[i] = data_lower_swapped - product;
                data_out_upper[precomputed_idx[i].idx_upper] = data_lower_swapped + product;
            }
            else
            {
                data_out_lower[i] = data_lower_swapped + product;
                data_out_upper[precomputed_idx[i].idx_upper] = data_lower_swapped - product;
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
        ButterflyOperator<TC_FFT, TC_FFT>(fft_input_lower, fft_input_upper, precomputed_idx[0], 0, fft_stage_lower[0], fft_stage_upper[0]);

    BUTTERFLY_OPERATOR_LOOP:
        for (int stage_num = 1; stage_num < n_clog2_c - 1; stage_num++)
        {
#pragma HLS unroll
            ButterflyOperator<TC_FFT, TC_FFT>(fft_stage_lower[stage_num - 1], fft_stage_upper[stage_num - 1], precomputed_idx[stage_num], stage_num, fft_stage_lower[stage_num], fft_stage_upper[stage_num]);
        }

        ButterflyOperator<TC_FFT, TC_FFT>(fft_stage_lower[n_clog2_c - 2], fft_stage_upper[n_clog2_c - 2], precomputed_idx[n_clog2_c - 1], n_clog2_c - 1, fft_output_lower, fft_output_upper);
    }
};
