#include <stdio.h>
#include <cstdlib>
#include "dft_sysdef.h"
#include "dft_wrapper.h"

int main()
{
    hls::stream<TR_INPUT_SIGNAL> input_signal_stream;
    TR_INPUT_SIGNAL input_signal;
    int input_signal_int;
    TR_DFT_NORM dft_magnitudes[N / 2];
    int sample_idx;
    // Read dat file
    fprintf(stdout, "START TEST\n");
    fprintf(stdout, "Reading data file\n");
    FILE *fp;
    fp = fopen("E:/git/hls_examples/dat/file_example_WAV_1MG.dat", "r");

    for (int i = 0; i < N; i++)
    {
        fscanf(fp, "%d %d\n", &sample_idx, &input_signal_int);
        //fprintf(stdout, "%d \t %d\n", sample_idx, input_signal_int);
        input_signal = TR_INPUT_SIGNAL(input_signal_int);
        input_signal_stream.write(input_signal);

        dft_wrapper(input_signal_stream, dft_magnitudes);
    }
    fclose(fp); // Close the file
    for (int i = 0; i < N / 2; i++)
    {
        fprintf(stdout, "dft_magnitudes[%d] = %d\n", i, int(dft_magnitudes[i]));
    }
    return 0;
}
