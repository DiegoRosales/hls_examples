#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include "fft_sysdef.h"
#include "fft_wrapper.h"

using namespace std;

double abs(TC_FFT data)
{
    return (sqrt(pow(data.real().to_double(), 2) + pow(data.imag().to_double(), 2)));
}

double calculatePercentRMS(double predicted[N], double actual[N])
{

    double mse = 0.0;
    double rms = 0.0;
    double mean = 0.0;

    for (size_t i = 0; i < N; i++)
    {
        mse += pow(predicted[i] - actual[i], 2);
        mean += actual[i];
    }

    mse /= N;  // Calculate the mean squared error
    mean /= N; // Calculate the mean

    rms = sqrt(mse); // Calculate the square root to get the RMSE

    // Calculate the range of the data

    double percentRMS = (rms / mean) * 100.0; // Calculate Percent RMS
    return percentRMS;
}

int main()
{
    hls::stream<TI_INPUT_SIGNAL> input_signal_stream;
    TI_INPUT_SIGNAL input_signal;
    int input_signal_int;
    TC_FFT fft_output[N];
    TB valid = 0;
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

    while (valid.to_int() == 0)
    {
        fscanf(fp, "%d %d\n", &sample_idx, &input_signal_int);
        input_signal = TI_INPUT_SIGNAL(input_signal_int);
        input_signal_stream.write(input_signal);
        fft_wrapper(input_signal_stream, fft_output, valid);
        samples_counter++;
    }

    fclose(fp); // Close the file

    fprintf(stdout, "INFO: %d samples sent\n", samples_counter);

    fp_generated = fopen("E:/git/hls_examples/dat/generated_data.dat", "w");
    fp_golden = fopen("E:/git/hls_examples/dat/file_example_WAV_1MG_golden_data.dat", "r");
    for (int i = 0; i < N; i++)
    {
        fscanf(fp_golden, "%lf\n", &fft_predicted_tmp);
        fft_predicted[i] = fft_predicted_tmp;
        fft_magnitud[i] = abs(fft_output[i]);
        fprintf(fp_generated, "%d %lf\n", i, fft_magnitud[i]);
    }

    fclose(fp_generated); // Close the file
    fclose(fp_golden);

    fprintf(stdout, "Calculating RMS%\n\n");
    rmse = calculatePercentRMS(fft_predicted, fft_magnitud);
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
