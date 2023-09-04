#include <stdio.h>
#include <cstdlib>
#include "dft_sysdef.h"
#include "dft_wrapper.h"

int main()
{
    hls::stream<TR_INPUT_SIGNAL> input_signal_stream;
    TR_INPUT_SIGNAL input_signal;
    double input_signal_double;
    TR_DFT_NORM dft_magnitudes[N / 2];
    int sample_idx;
    // Read dat file
    fprintf(stdout, "START TEST\n");
    fprintf(stdout, "Reading data file\n");
    FILE *fp;
    //fp = fopen("E:/git/hls_examples/dat/file_example_WAV_1MG.dat", "r");
    //fp = fopen("E:/git/hls_examples/dat/sin20khz.dat", "r");
    fp = fopen("E:/git/hls_examples/dat/sin1khz.dat", "r");

    for (int i = 0; i < N; i++)
    {
        fscanf(fp, "%d %lf\n", &sample_idx, &input_signal_double);
        // fprintf(stdout, "input_signal_double[%d] = %lf\n", sample_idx, input_signal_double);
        input_signal = TR_INPUT_SIGNAL(input_signal_double);
        // fprintf(stdout, "input_signal_fixed[%d] = %lf\n", sample_idx, input_signal.to_double());
        input_signal_stream.write(input_signal);

        dft_wrapper(input_signal_stream, dft_magnitudes);
    }
    fclose(fp); // Close the file
    for (int i = 0; i < N / 2; i++)
    {
        // fprintf(stdout, "dft_magnitudes[%d] = %lf\n", i, dft_magnitudes[i].to_double());
        fprintf(stdout, "%lf,\n", dft_magnitudes[i].to_double());
    }
    return 0;
}
