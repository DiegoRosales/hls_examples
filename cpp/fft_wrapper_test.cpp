#include <stdio.h>
#include <cstdlib>
#include "fft_sysdef.h"
#include "fft_wrapper.h"

int main()
{
    hls::stream<TR_INPUT_SIGNAL> input_signal_stream;
    TR_INPUT_SIGNAL input_signal;
    double input_signal_double;
    TR_FFT_NORM fft_magnitudes[N];
    int sample_idx;
    TB start = 0;
    TB done;
    // Read dat file
    fprintf(stdout, "START TEST\n");
    fprintf(stdout, "Reading data file\n");
    FILE *fp;
    fp = fopen("E:/git/hls_examples/dat/file_example_WAV_1MG.dat", "r");
    // fp = fopen("E:/git/hls_examples/dat/sin20khz.dat", "r");
    // fp = fopen("E:/git/hls_examples/dat/sin1khz.dat", "r");

    for (int i = 0; i < N; i++)
    {
        if (i == 0)
        {
            start = 1;
        }
        fscanf(fp, "%d %lf\n", &sample_idx, &input_signal_double);
        input_signal = TR_INPUT_SIGNAL(input_signal_double);
        // fprintf(stdout, "input_signal_double[%d] = %lf = %lf\n", sample_idx, input_signal_double, input_signal.to_double());
        // fprintf(stdout, "input_signal_fixed[%d] = %lf\n", sample_idx, input_signal.to_double());
        input_signal_stream.write(input_signal);

        fft_wrapper(input_signal_stream, start, fft_magnitudes, done);
        fprintf(stdout, "start = %d\n", start);
    }

    while (done == 0)
    {
        fft_wrapper(input_signal_stream, start, fft_magnitudes, done);
        fprintf(stdout, "done = %d\n", done);
    }
    fprintf(stdout, "start (end) = %d\n", start);

    fclose(fp); // Close the file
    for (int i = 0; i < N; i++)
    {
        fprintf(stdout, "fft_magnitudes[%d] = %lf,\n", i, fft_magnitudes[i].to_double());
    }
    return 0;
}
