#include "ap_fixed.h"
#include "dft_sysdef.h"
#include "dft.h"
#include "hls_stream.h"

void dft_wrapper(
    // Inputs
    hls::stream<TR_INPUT_SIGNAL> &input_signal,
    // Outputs
    TR_DFT_NORM dft_magnitudes[N / 2]);
