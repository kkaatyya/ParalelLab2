#include <iostream>
#include <vector>
#include <random>
#include <climits>
#include <chrono>
#include <iomanip>

using namespace std;
using chrono::high_resolution_clock;
using chrono::duration_cast;
using chrono::nanoseconds;

void fillArray(vector<int>& numbers)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(INT_MIN, INT_MAX);

    for (int& num : numbers)
    {
        num = dist(gen);
    }
}

void calculateXORByModulo15(int startIndex, int endIndex, const vector<int>& numbers, int& xorResult)
{
    xorResult = 0;
    for (int i = startIndex; i < endIndex; ++i)
    {
        if (numbers[i] % 15 == 0)
        {
            xorResult ^= numbers[i];
        }
    }
}

void executeSequentially(const vector<int>& numbers, int& xorResult)
{
    calculateXORByModulo15(0, numbers.size(), numbers, xorResult);
}

int main()
{
    vector<int> matrixSizes = { 10000, 1000000, 100000000 };

    cout << "\n=========================== XOR Test Results ============================" << endl;
    cout << left << setw(15) << "Matrix Size" << setw(15) << "Mode" << setw(20) << "Time (s)" << "XOR Result" << endl;
    cout << "-------------------------------------------------------------------------" << endl;

    for (int matrixSize : matrixSizes)
    {
        vector<int> numbers(matrixSize);
        fillArray(numbers);

        int xorResult = 0;
        auto start = high_resolution_clock::now();
        executeSequentially(numbers, xorResult);
        auto end = high_resolution_clock::now();
        double elapsed = duration_cast<nanoseconds>(end - start).count() * 1e-9;

        cout << left << setw(15) << matrixSize
            << setw(15) << "Sequential"
            << setw(20) << fixed << setprecision(6) << elapsed
            << xorResult << endl;
    }

    cout << "===========================================================================" << endl;
    return 0;
}
