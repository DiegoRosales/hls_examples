constexpr double pi = 3.14159265358979323846;

using Complex = std::complex<double>;

// Bit-reversal permutation function
void bit_reverse_permutation(std::vector<Complex>& data) {
    // ... (same as previous code)
}

void cooley_tukey_fft(std::vector<Complex>& data) {
    const int N = data.size();
    bit_reverse_permutation(data);

    // Precompute twiddle factors and store them in a table
    std::vector<Complex> twiddle_factors(N);
    for (int k = 0; k < N; ++k) {
        double angle = -2.0 * pi * k / N;
        twiddle_factors[k] = std::polar(1.0, angle);
    }

    for (int step = 2; step <= N; step *= 2) {
        int subarray_size = step / 2;

        for (int k = 0; k < N; k += step) {
            Complex w = 1.0;
            for (int j = 0; j < subarray_size; ++j) {
                Complex t = w * data[k + j + subarray_size];
                Complex u = data[k + j];
                data[k + j] = u + t;
                data[k + j + subarray_size] = u - t;
                w *= twiddle_factors[N / step * j]; // Lookup twiddle factor
            }
        }
    }
}