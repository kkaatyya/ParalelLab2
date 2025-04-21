#include <iostream>
#include <vector>
#include <random>
#include <climits>
#include <chrono>
#include <iomanip>
#include <thread>
#include <mutex>

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

void parallelWithMutex(const vector<int>& numbers, int& xorResult, int threadCount)
{
    mutex mtx;
    xorResult = 0;
    vector<thread> threads;
    int elementsPerThread = numbers.size() / threadCount;

    for (int i = 0; i < threadCount; i++)
    {
        int startIndex = i * elementsPerThread;
        int endIndex = (i == threadCount - 1) ? numbers.size() : startIndex + elementsPerThread;
        threads.emplace_back([&, startIndex, endIndex]() {
            int localXor = 0;
            calculateXORByModulo15(startIndex, endIndex, numbers, localXor);
            lock_guard<mutex> lock(mtx);
            xorResult ^= localXor;
            });
    }

    for (auto& th : threads) th.join();
}

void parallelWithCAS(const vector<int>& numbers, int& xorResult, int threadCount)
{
    atomic<int> atomicXor(0);
    vector<thread> threads;
    int elementsPerThread = numbers.size() / threadCount;

    for (int i = 0; i < threadCount; i++)
    {
        int startIndex = i * elementsPerThread;
        int endIndex = (i == threadCount - 1) ? numbers.size() : startIndex + elementsPerThread;
        threads.emplace_back([&, startIndex, endIndex]() {
            for (int j = startIndex; j < endIndex; ++j)
            {
                if (numbers[j] % 15 == 0)
                {
                    int expected, desired;
                    do {
                        expected = atomicXor.load();
                        desired = expected ^ numbers[j];
                    } while (!atomicXor.compare_exchange_weak(expected, desired));
                }
            }
            });
    }

    for (auto& th : threads) th.join();
    xorResult = atomicXor.load();
}

int main()
{
    vector<int> matrixSizes = { 10000, 1000000, 100000000,  500000000, };
    const int threadCount = 8;

    cout << "\n=========================== XOR Test Results ============================" << endl;
    cout << left << setw(15) << "Matrix Size" << setw(15) << "Mode" << setw(20) << "Time (s)" << "XOR Result" << endl;
    cout << "-------------------------------------------------------------------------" << endl;

    for (int matrixSize : matrixSizes)
    {
        vector<int> numbers(matrixSize);
        fillArray(numbers);

        {
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

        {
            int xorResult = 0;
            auto start = high_resolution_clock::now();
            parallelWithMutex(numbers, xorResult, threadCount);
            auto end = high_resolution_clock::now();
            double elapsed = duration_cast<nanoseconds>(end - start).count() * 1e-9;

            cout << left << setw(15) << matrixSize
                << setw(15) << "Mutex"
                << setw(20) << fixed << setprecision(6) << elapsed
                << xorResult << endl;
        }

        {
            int xorResult = 0;
            auto start = high_resolution_clock::now();
            parallelWithCAS(numbers, xorResult, 8);
            auto end = high_resolution_clock::now();
            double elapsed = duration_cast<nanoseconds>(end - start).count() * 1e-9;

            cout << left << setw(15) << matrixSize
                << setw(15) << "CAS"
                << setw(20) << fixed << setprecision(6) << elapsed
                << xorResult << endl;
        }
    }



    cout << "===========================================================================" << endl;
    return 0;
}
