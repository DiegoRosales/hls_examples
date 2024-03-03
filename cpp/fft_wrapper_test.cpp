/*
    File: fft_wrapper_test.cpp
    Author: Lucas Mariano Grigolato
    Date: 2024/02/25
    Description: This file contains the test case for the wrapper function
    of the Radix-2 FFT HLS block. It reads input data from a file, processes
    it through the FFT wrapper function, compares the results with golden
    output data, and calculates the Root Mean Squared Error (RMSE) percentage
    to assess the accuracy of the implementation. The test is conducted
    iteratively for a specified number of iterations or until the RMSE exceeds
    a predefined threshold.
*/

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
    // Variable definitions
    hls::stream<TI_INPUT_SIGNAL> input_signal_stream; // Stream for input signal
    hls::stream<TC_FFT> fft_output_stream;            // Stream for input signal
    TI_INPUT_SIGNAL input_sample;                     // Single input sample
    int input_sample_int;                             // Temporary storage for input sample integer value
    TC_FFT fft_input_lower[N / 2];                    // Lower part of FFT input
    TC_FFT fft_input_upper[N / 2];                    // Upper part of FFT input
    TC_FFT fft_output[N];                             // Full FFT output
    double fft_magnitude_test[N];                     // Magnitude of FFT output for testing
    double fft_magnitude_golden[N];                   // Magnitude of golden FFT output
    int samples_counter;                              // Counter for the number of samples read
    double rmse_last;                                 // Last calculated RMSE%
    double rmse[num_iterations];                      // Array to store RMSE% values for each iteration
    double fft_magnitude_golden_tmp;                  // Temporary storage for golden FFT magnitude

    // Relevant file paths
    string root_path = "../../../../../../";
    string dat_path = root_path + "dat/";
    string ref_input_filepath = dat_path + "file_example_WAV_1MG.dat";  // Path to input data file
    string test_output_filepath = dat_path + "fft_test_output.dat";     // Path to test output data file
    string golden_output_filepath = dat_path + "fft_golden_output.dat"; // Path to golden output data file

    // File pointers
    FILE *fp_ref_input;     // Pointer to input data file
    FILE *fp_test_output;   // Pointer to test output data file
    FILE *fp_golden_output; // Pointer to golden output data file

    // Open relevant files
    fp_ref_input = fopen(ref_input_filepath.c_str(), "r");         // Open input data file in read mode
    fp_test_output = fopen(test_output_filepath.c_str(), "w");     // Open test output data file in write mode
    fp_golden_output = fopen(golden_output_filepath.c_str(), "r"); // Open golden output data file in read mode

    // Read data from the input file
    fprintf(stdout, "\n\nINFO: Starting test\n\n");
    fprintf(stdout, "#########################################################\n\n");
    for (int i = 0; i < num_iterations; i++)
    {
        fprintf(stdout, "INFO: Iteration number = %d\n\n", i);
        fprintf(stdout, "INFO: Reading data file\n\n");

        // Write input samples into input AXI-Stream channel
        samples_counter = 0;
        while (samples_counter < N)
        {
            fscanf(fp_ref_input, "%d\n", &input_sample_int);  // Read input signal integer value from file
            input_sample = TI_INPUT_SIGNAL(input_sample_int); // Convert integer to input signal type
            input_signal_stream.write(input_sample);          // Write input signal to input stream
            samples_counter++;
        }

        fprintf(stdout, "INFO: Sending samples to input channel and computing FFT\n");

        // Run HLS block wrapper function
        fft_wrapper(
            // Inputs
            input_signal_stream,
            // Outputs
            fft_output_stream);

        // Map the result into a single array for simplification
        for (int i = 0; i < N; i++)
        {
            fft_output[i] = fft_output_stream.read();
        }

        fprintf(stdout, "INFO: %d samples sent\n\n", samples_counter);

        // Read the golden output file and calculate the magnitude of the complex values obtained from the calculated FFT.
        for (int j = 0; j < N; j++)
        {
            fscanf(fp_golden_output, "%lf\n", &fft_magnitude_golden_tmp); // Read golden FFT magnitude from file
            fft_magnitude_golden[j] = fft_magnitude_golden_tmp;           // Store golden FFT magnitude
            fft_magnitude_test[j] = abs<TC_FFT>(fft_output[j]);           // Calculate magnitude of test FFT output
            fprintf(fp_test_output, "%lf\n", fft_magnitude_test[j]);      // Write test FFT magnitude to file
        }

        // Calculate the Root Mean Squared Error (RMSE) percentage between the golden and the test data.
        fprintf(stdout, "INFO: Calculating RMSE%\n");
        rmse[i] = calculatePercentRMSE<N>(fft_magnitude_golden, fft_magnitude_test); // Calculate RMSE percentage
        fprintf(stdout, "INFO: RMSE%% = %lf%%\n\n", rmse[i]);                        // Print RMSE percentage
        fprintf(stdout, "#########################################################\n\n");

        rmse_last = rmse[i]; // Store last calculated RMSE

        // Break if RMSE exceeds threshold
        if (rmse_last > rmse_threshold)
        {
            break;
        }
    }

    // Close previously opened files
    fclose(fp_ref_input);
    fclose(fp_test_output);
    fclose(fp_golden_output);

    // Print test result
    if (rmse_last < rmse_threshold)
    {
        fprintf(stdout, "\nINFO: Test passed successfully!!!\n\n");
        return 0; // Return success
    }
    else
    {
        fprintf(stdout, "\nERROR: Test failed :(\n\n");
        return 1; // Return failure
    }
}
