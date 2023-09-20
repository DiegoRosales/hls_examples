#include <stdio.h>
#include <cstdlib>
#include "fft_sysdef.h"
#include "fft_wrapper.h"

int main()
{
    hls::stream<TI_INPUT_SIGNAL> input_signal_stream;
    TI_INPUT_SIGNAL input_signal;
    int input_signal_int;
    TC_FFT fft_output[N];
    double fft_magnitud;
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
        fscanf(fp, "%d %d\n", &sample_idx, &input_signal_int);
        input_signal = TI_INPUT_SIGNAL(input_signal_int);
        input_signal_stream.write(input_signal);

        fft_wrapper(input_signal_stream, start, fft_output, done);
    }

    while (done == 0)
    {
        fft_wrapper(input_signal_stream, start, fft_output, done);
    }

    fclose(fp); // Close the file

    fp = fopen("E:/git/hls_examples/dat/generated_data.dat", "w");
    for (int i = 0; i < N; i++)
    {
        fft_magnitud = fft_output[i].real().to_double() * fft_output[i].real().to_double() + fft_output[i].imag().to_double() * fft_output[i].imag().to_double();
        fprintf(fp, "%d %lf\n", i, fft_magnitud);
    }
    return 0;
}
