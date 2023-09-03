#include "dft_wrapper.h"

void dft_wrapper(
    // Inputs
    hls::stream<TR_INPUT_SIGNAL> &input_signal,
    // Outputs
    TR_DFT_NORM dft_magnitudes[N / 2])
{
    static dft dft_obj;
    dft_obj.computeDFTMagnitude(input_signal, dft_magnitudes);
}
