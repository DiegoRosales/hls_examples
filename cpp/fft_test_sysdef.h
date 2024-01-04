template <class TC>
double abs(TC data)
{
    return (sqrt(pow(data.real().to_double(), 2) + pow(data.imag().to_double(), 2)));
}

template <int N>
double calculatePercentRMS(double predicted[N], double actual[N])
{

    double mse = 0.0;
    double rms = 0.0;
    double mean = 0.0;

    for (size_t i = 0; i < N; i++)
    {
        mse += pow(predicted[i] - actual[i], 2);
        mean += actual[i];
    }

    mse /= N;  // Calculate the mean squared error
    mean /= N; // Calculate the mean

    rms = sqrt(mse); // Calculate the square root to get the RMSE

    // Calculate the range of the data

    double percentRMS = (rms / mean) * 100.0; // Calculate Percent RMS
    return percentRMS;
}
