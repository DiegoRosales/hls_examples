#include <cmath>
#include <complex>
#include "dft_sysdef.h"
#include "hls_stream.h"

class dft
{
public:
    // Constructor
    dft(void)
    {
        reset();
    }

    // Destructor
    ~dft(void) {}

    TC_TWIDDLE_FACTOR twiddle_factors[N]; // Pre-computed twiddle factors

    // Reset function
    void reset()
    {
        std::complex<double> twiddle_factor_k;
        sample_idx = 0;
        // Initialize the twiddle factors and arrays
        for (int k = 0; k < N; k++)
        {
            twiddle_factor_k = std::exp(std::complex<double>(0, -2.0 * M_PI * k / N));
            twiddle_factors[k] = std::complex<TR_TWIDDLE_FACTOR>(twiddle_factor_k.real(), twiddle_factor_k.imag());
        }
        for (int n = 0; n < N; n++)
        {
            sample_array[n] = 0;
        }
    }

    void computeDFTMagnitude(
        // Inputs
        hls::stream<TR_INPUT_SIGNAL> &input_signal,
        // Outputs
        TR_DFT_NORM dft_magnitudes[N / 2])
    {
        TR_INPUT_SIGNAL input_signal_tmp = 0;
        if (input_signal.read_nb(input_signal_tmp))
        {
            for (int n = 0; n < N - 1; ++n)
            {
                sample_array[n] = sample_array[n + 1];
            }
            sample_array[N] = input_signal_tmp;
        }

        for (int k = 0; k < N / 2; k++)
        {
            dft_result[k] = 0;
        }

        for (int k = 0; k < N / 2; ++k)
        {
            for (int n = 0; n < N; ++n)
            {
                dft_result[k] = TC_DFT(dft_result[k].real() + sample_array[n] * twiddle_factors[(k * n) % N].real(),
                                       dft_result[k].imag() + sample_array[n] * twiddle_factors[(k * n) % N].imag());
            }
        }
        for (int k = 0; k < N / 2; ++k)
        {
            dft_magnitudes[k] = TR_DFT_NORM((dft_result[k].real() * dft_result[k].real() + dft_result[k].imag() * dft_result[k].imag()) >> n_clog2_c);
        }
    }

private:
    TR_INPUT_SIGNAL sample_array[N];
    ap_uint<n_clog2_c> sample_idx;
    TC_DFT dft_result[N / 2];
};
