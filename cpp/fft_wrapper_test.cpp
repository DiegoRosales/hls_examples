#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include "fft_sysdef.h"
#include "fft_test_sysdef.h"
#include "fft_wrapper.h"

using namespace std;

int main()
{
    // Variables definitions
    hls::stream<TI_INPUT_SIGNAL> input_signal_stream;
    TI_INPUT_SIGNAL input_signal;
    TC_FFT fft_input_lower[N / 2];
    TC_FFT fft_input_upper[N / 2];
    int input_signal_int;
    TC_FFT fft_output[N];
    double fft_magnitude_test[N];
    double fft_magnitude_golden[N];
    int samples_counter;
    double rmse;
    double fft_magnitude_golden_tmp;

    // Relevant paths
    string root_path = "../../../../../../";
    string dat_path = root_path + "dat/";
    string ref_input_filepath = dat_path + "file_example_WAV_1MG.dat";
    string test_output_filepath = dat_path + "fft_test_output.dat";
    string golden_output_filepath = dat_path + "fft_golden_output.dat";

    // File pointers
    FILE *fp_ref_input;
    FILE *fp_test_output;
    FILE *fp_golden_output;

    // Open relevant files
    fp_ref_input = fopen(ref_input_filepath.c_str(), "r");
    fp_test_output = fopen(test_output_filepath.c_str(), "w");
    fp_golden_output = fopen(golden_output_filepath.c_str(), "r");

    // Read dat file
    fprintf(stdout, "\n\nINFO: Starting test\n");
    fprintf(stdout, "INFO: Reading data file\n\n");

    // Write input samples into input AXI-Stream channel
    samples_counter = 0;
    while (samples_counter < N)
    {
        fscanf(fp_ref_input, "%d\n", &input_signal_int);
        input_signal = TI_INPUT_SIGNAL(input_signal_int);
        input_signal_stream.write(input_signal);
        samples_counter++;
    }

    fprintf(stdout, "INFO: Sending samples to input channel and computing FFT\n");

    // Run HLS block wrapper function
    fft_wrapper(
        // Inputs
        input_signal_stream,
        // Outputs
        fft_output);

    fprintf(stdout, "INFO: %d samples sent\n\n", samples_counter);

    // Read the golden output file and calculate the magnitude of the complex values obtained from the calculated FFT.
    for (int i = 0; i < N; i++)
    {
        fscanf(fp_golden_output, "%lf\n", &fft_magnitude_golden_tmp);
        fft_magnitude_golden[i] = fft_magnitude_golden_tmp;
        fft_magnitude_test[i] = abs<TC_FFT>(fft_output[i]);
        fprintf(fp_test_output, "%lf\n", fft_magnitude_test[i]); // Write test output in file for future use
    }

    // Close previously opened files
    fclose(fp_ref_input);
    fclose(fp_test_output);
    fclose(fp_golden_output);

    // Calculate the Root Mean Squared Error (RMSE) percentage between the golden and the test data.
    fprintf(stdout, "INFO: Calculating RMSE%\n");
    rmse = calculatePercentRMSE<N>(fft_magnitude_golden, fft_magnitude_test);
    fprintf(stdout, "INFO: RMSE%% = %lf%%\n\n", rmse);

    // Define the accepted upper threshold for RMSE percentage.
    double rmse_threshold = 1; // %

    if (rmse < rmse_threshold)
    {
        fprintf(stdout, "INFO: Test passed successfully!!!\n\n");
        return (0);
    }
    else
    {
        fprintf(stdout, "ERROR: Test failed :(\n\n");
        return (1);
    }

    return 0;
}
