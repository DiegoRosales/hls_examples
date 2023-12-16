#include "fft_wrapper.h"

TUI_SAMPLE_ARRAY_IDX sample_count = 0;
TB buffer_full[n_instances_c];

void fft_wrapper(
    // Inputs
    hls::stream<TI_INPUT_SIGNAL> &input_signal,
    // Outputs
    TC_FFT fft_output[N])
{
    TI_INPUT_SIGNAL sample_in;
    TC_FFT fft_input[n_instances_c][N];
    TC_FFT fft_output_tmp[n_instances_c][N];

    static fft<N, n_clog2_c> fft_obj[n_clog2_c];

    if (input_signal.read_nb(sample_in))
    {
        fft_input[sample_count / N][fft_obj[0].bit_reversed_idx[sample_count % N]] = TC_FFT(TR_FFT(sample_in), 0);
        fprintf(stdout, "sample_count = %d\n", sample_count);
        fprintf(stdout, "sample_count % N = %d\n", sample_count % N);
        fprintf(stdout, "stage_n = %d\n", sample_count / N);
        fprintf(stdout, "fft_input[%d][%d] = %lf\n", sample_count / N, fft_obj[0].bit_reversed_idx[sample_count % N], fft_input[sample_count / N][fft_obj[0].bit_reversed_idx[sample_count % N]].real().to_double());
        if (sample_count % N == N - 1)
        {
            buffer_full[sample_count / N] = 1;
            fprintf(stdout, "buffer %d set to 1\n", sample_count / N);
            fprintf(stdout, "sample_count = %d\n", sample_count);
        }
        else
        {
            buffer_full[sample_count / N] = 0;
            fprintf(stdout, "buffer %d set to 0\n", sample_count / N);
        }
        sample_count++;
    }
    for (int i = 0; i < n_instances_c; i++)
    {
        if (buffer_full[i])
        {
            fprintf(stdout, "Calculating FFT for stage %d\n", i);
            fprintf(stdout, "buffer_full[0] = %d\n", buffer_full[0]);
            fprintf(stdout, "buffer_full[1] = %d\n", buffer_full[1]);
            fprintf(stdout, "buffer_full[2] = %d\n", buffer_full[2]);
            fprintf(stdout, "buffer_full[3] = %d\n", buffer_full[3]);
            for (int j = 0; j < N; j++)
            {
                fprintf(stdout, "fft_input[%d][%d] = %lf\n", i, j, fft_input[i][j].real().to_double());
            }
            fft_obj[i].doFFT(fft_input[i], fft_output_tmp[i]);
            buffer_full[i] = 0;
        }
    }
    for (int j = 0; j < n_instances_c; j++)
    {
        if (fft_obj[j].done)
        {
            fprintf(stdout, "done[0] = %d\n", fft_obj[0].done);
            fprintf(stdout, "done[1] = %d\n", fft_obj[1].done);
            fprintf(stdout, "done[2] = %d\n", fft_obj[2].done);
            fprintf(stdout, "done[3] = %d\n", fft_obj[3].done);
            for (int i = 0; i < N; i++)
            {
                // fprintf(stdout, "fft_output_tmp[%d][%d] = %lf + j%lf\n", j, i, fft_output_tmp[j][i].real().to_double(), fft_output_tmp[j][i].imag().to_double());
                fft_output[i] = fft_output_tmp[j][i];
                fft_obj[j].done = 0;
            }
            break; // Exit the loop once a match is found
        }
    }
}