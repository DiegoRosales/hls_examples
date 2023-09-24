#include "ap_fixed.h"
#include "hls_stream.h"

static const int N = 1024;
static const int n_clog2_c = 10;
typedef ap_fixed<16, 1> TR_INPUT_SIGNAL;
typedef ap_fixed<16, 1> TR_TWIDDLE_FACTOR;
typedef std::complex<TR_TWIDDLE_FACTOR> TC_TWIDDLE_FACTOR;
typedef ap_fixed<n_clog2_c * 2, n_clog2_c> TR_DFT;
typedef std::complex<TR_DFT> TC_DFT;
typedef ap_ufixed<n_clog2_c * 2, n_clog2_c> TR_DFT_NORM;
typedef ap_uint<1> TB;

void dft_wrapper(
    // Inputs
    hls::stream<TR_DFT_NORM> &input_signal,
    // Outputs
    TR_DFT_NORM dft_magnitudes[N / 2])
{
    // This is just a placeholder to get the interfaces
    static int i = 0;
    dft_magnitudes[i] = input_signal.read();
    if (i < (N / 2)) i++;
    else i = 0;
}
