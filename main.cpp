#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <atomic>
#include <string>
#include <iostream>
// ������ �������
const int ARRAY_SIZE = 1000000;

// ��������� ��� �������� ������ � ������
struct ThreadData {
    int* array;
    int start;
    int end;
    std::atomic<long long>* result;
};

// ������� ��� ��������� ������� ��������� �����
void generateArray(int* array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 100 + 1;  // ��������� ����� �� 1 �� 100
    }
}

// ������� ��� ���������� ����� ����� �������
void calculatePartialSum(ThreadData* data) {
    long long partialSum = 0;
    for (int i = data->start; i < data->end; i++) {
        partialSum += data->array[i];
    }
    // ���������� atomic ��� ���������� ������ ����������
    data->result->fetch_add(partialSum, std::memory_order_relaxed);
}

// ������� ��� ������������� ���������� �����
long long singleThreadSum(int* array, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    return sum;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <number_of_threads>" << std::endl;
        return 1;
    }

    int numThreads = std::stoi(argv[1]);
    if (numThreads <= 0) {
        std::cerr << "Number of threads must be positive." << std::endl;
        return 1;
    }

    // ������������� �������
    int* array = new int[ARRAY_SIZE];
    generateArray(array, ARRAY_SIZE);

    // ����� ���������� � ���������� ������� � ������� �������
    std::cout << "Number of threads: " << numThreads << std::endl;
    std::cout << "Array size: " << ARRAY_SIZE << std::endl;

    // ������������ ������
    auto startParallel = std::chrono::high_resolution_clock::now();
    std::thread* threads = new std::thread[numThreads];
    std::atomic<long long> parallelSum(0);

    int chunkSize = ARRAY_SIZE / numThreads;
    ThreadData* threadData = new ThreadData[numThreads];

    for (int i = 0; i < numThreads; i++) {
        threadData[i].array = array;
        threadData[i].start = i * chunkSize;
        threadData[i].end = (i == numThreads - 1) ? ARRAY_SIZE : (i + 1) * chunkSize;
        threadData[i].result = &parallelSum;
        threads[i] = std::thread(calculatePartialSum, &threadData[i]);
    }

    for (int i = 0; i < numThreads; i++) {
        threads[i].join();
    }
    auto endParallel = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallelDuration = endParallel - startParallel;

    // ������������ ������
    auto startSingle = std::chrono::high_resolution_clock::now();
    long long singleSum = singleThreadSum(array, ARRAY_SIZE);
    auto endSingle = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> singleDuration = endSingle - startSingle;

    // ����� �����������
    std::cout << "Parallel computation result: " << parallelSum.load() << std::endl;
    std::cout << "Single-threaded computation result: " << singleSum << std::endl;
    std::cout << "Time taken (parallel): " << parallelDuration.count() * 1000 << " ms" << std::endl;
    std::cout << "Time taken (single-threaded): " << singleDuration.count() * 1000 << " ms" << std::endl;

    // �������� ������������
    if (parallelSum.load() == singleSum) {
        std::cout << "Sum is correct!" << std::endl;
    }
    else {
        std::cout << "Sum is incorrect!" << std::endl;
    }

    // ������� ������
    delete[] array;
    delete[] threads;
    delete[] threadData;

    return 0;
}
