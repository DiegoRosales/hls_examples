#include <iostream>
#include <random>
#include "average_wrapper.h"

int main()
{
    ap_uint<16> inputs[N];            // array to store random integers
    ap_ufixed<24, 16> average_result; // Variable to store result

    // Seed for random number generation
    std::random_device rd;
    std::mt19937 gen(42);

    // Define the distribution for the random integers
    std::uniform_int_distribution<> dis(1, 100); // Range from 1 to 100, change as needed

    // Fill the array with random integers
    for (int i = 0; i < N; i++)
    {
        inputs[i] = ap_uint<16>(dis(gen));
    }

    // Calculate the sum of all elements
    int sum = 0;
    for (int i = 0; i < N; ++i)
    {
        sum += inputs[i].to_int();
    }

    // Calculate the average
    double average_golden = static_cast<double>(sum) / N;

    // Execute wrapper function
    average_wrapper(
        // Inputs
        inputs,
        // Outputs
        average_result);

    if (average_golden == average_result.to_double())
    {
        std::cout << "INFO: Test passed successfully!!!\n";
        std::cout << "average_golden = " << average_golden << ", average_result = " << average_result.to_double() << "\n";
        return 0;
    }
    else
    {
        std::cout << "ERROR: Test failed\n";
        std::cout << "average_golden = " << average_golden << ", average_result = " << average_result.to_double() << "\n";
        return 1;
    }
}