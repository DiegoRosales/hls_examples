// Number of iterations for testbench
static const int num_iterations = 20;

// Define the accepted upper threshold for RMSE percentage.
double rmse_threshold = 1; // %

// Function to calculate the absolute value or modulus of a complex number.
template <class TC>
double abs(TC data)
{
    return (sqrt(pow(data.real().to_double(), 2) + pow(data.imag().to_double(), 2)));
}

// Function to calculate the Root Mean Squared Error Percentage (RMSE%) between two sets of data.
template <int N>
double calculatePercentRMSE(double predicted[N], double actual[N])
{
    double mse = 0.0;
    double rmse = 0.0;
    double mean = 0.0;

    for (int i = 0; i < N; i++)
    {
        mse += pow(predicted[i] - actual[i], 2);
        mean += actual[i];
    }

    mse /= N;  // Calculate the mean squared error (MSE)
    mean /= N; // Calculate the mean

    rmse = sqrt(mse); // Calculate the square root to obtain the RMSE

    double percentRMSE = (rmse / mean) * 100.0; // Calculate the RMSE%
    return percentRMSE;
}
