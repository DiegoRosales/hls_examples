#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include "fft_sysdef.h"
#include "fft_test_sysdef.h"
#include "fft_wrapper.h"

using namespace std;

int main()
{
    hls::stream<TI_INPUT_SIGNAL> input_signal_stream;
    TI_INPUT_SIGNAL input_signal;
    TC_FFT fft_input_lower[N / 2];
    TC_FFT fft_input_upper[N / 2];
    int input_signal_int;
    TC_FFT fft_output[N];
    double fft_magnitud[N];
    double fft_predicted[N];
    int sample_idx;
    int samples_counter;
    double rmse;
    double fft_predicted_tmp;
    FILE *fp;
    FILE *fp_generated;
    FILE *fp_golden;

    // Read dat file
    fprintf(stdout, "START TEST\n");
    fprintf(stdout, "Reading data file\n");

    fp = fopen("E:/git/hls_examples/dat/file_example_WAV_1MG.dat", "r");
    // fp = fopen("E:/git/hls_examples/dat/sin20khz.dat", "r");
    // fp = fopen("E:/git/hls_examples/dat/sin1khz.dat", "r");

    samples_counter = 0;
    while (samples_counter < N)
    {
        fscanf(fp, "%d %d\n", &sample_idx, &input_signal_int);
        input_signal = TI_INPUT_SIGNAL(input_signal_int);
        input_signal_stream.write(input_signal);
        samples_counter++;
    }

    fprintf(stdout, "Sending samples to input channel and computing FFT...\n");

    fft_wrapper(
        // Inputs
        input_signal_stream,
        // Outputs
        fft_output);

    fclose(fp); // Close the file

    fprintf(stdout, "INFO: %d samples sent\n", samples_counter);

    fp_generated = fopen("E:/git/hls_examples/dat/generated_data.dat", "w");
    fp_golden = fopen("E:/git/hls_examples/dat/file_example_WAV_1MG_golden_data.dat", "r");
    for (int i = 0; i < N; i++)
    {
        fscanf(fp_golden, "%lf\n", &fft_predicted_tmp);
        fft_predicted[i] = fft_predicted_tmp;
        fft_magnitud[i] = abs<TC_FFT>(fft_output[i]);
        fprintf(fp_generated, "%d %lf\n", i, fft_magnitud[i]);
    }

    fclose(fp_generated); // Close the file
    fclose(fp_golden);

    fprintf(stdout, "Calculating RMS%\n\n");
    rmse = calculatePercentRMS<N>(fft_predicted, fft_magnitud);
    fprintf(stdout, "RMS% = %lf%%\n\n", rmse);

    double tolerance;

    if (rmse < 1.0)
    {
        fprintf(stdout, "PASS!\n\n");
        return (0);
    }
    else
    {
        fprintf(stdout, "FAILED\n\n");
        return (1);
    }

    return 0;
}
