#include <thread>
#include <iostream>
#include <ctime>
#include <fstream>
#include <cmath>
#include <vector>

template <class T>
T* load_array(std::string file_name, size_t array_size) {
    T* result = new T[array_size];
    std::ifstream file;
    file.open (file_name, std::ios::binary);
    int i = 0;
    while(!file.eof()) {
        int buf = 0;
        file >> buf;
        result[i] = buf;
        i++;
    }
    return result;
}

template <class T>
void thread_func(const T* first_matrix, const T* second_matrix, T* result_matrix, int start_index, int end_index) {
    for (int i = start_index; i < end_index; ++i) {
        result_matrix[i] = first_matrix[i] + second_matrix[i];
    }
}

template <class T>
void calculation(size_t array_size, std::string first_matrix_filename,
                 std::string second_matrix_filename, int threads_count) {
    std::cout << "Initing arrays " << std::endl;
    std::cout << "Loading array from " << first_matrix_filename + ".txt" << std::endl;
    T* matrix_1 = load_array<T>(first_matrix_filename + ".txt", array_size);
    std::cout << "Loading array from " << second_matrix_filename + ".txt" << std::endl;
    T* matrix_2 = load_array<T>(second_matrix_filename + ".txt", array_size);
    std::cout << "Initing result matrix" << std::endl;
    T* result_matrix = new T[array_size];
    std::cout << "Arrays inited" << std::endl;
    size_t i = 0;
    std::cout << "Calculation starting " << std::endl;
    clock_t begin = clock();
    int count_per_thread = ceil(double(array_size) / double(threads_count));
    std::vector<std::thread> threads;
    for (int thread_index = 0; thread_index < threads_count; ++thread_index) {
        int start_index = thread_index * count_per_thread;
        int end_index = (thread_index + 1) * count_per_thread;
        end_index = end_index > array_size ? array_size : end_index;

        threads.push_back(std::thread(thread_func<T>, matrix_1, matrix_2, result_matrix, start_index, end_index));
    }
    for (std::thread& thread : threads) {
        thread.join();
    }

    clock_t end = clock();
    std::cout << "Calculation finished " << std::endl;
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Starting record result matrix to result.txt" << std::endl;
    std::ofstream output;
    output.open("result.txt");
    for (int j = 0; j < array_size; ++j) {
        output << result_matrix[j] << " ";
    }
    std::cout << "Result recorded to result.txt" << std::endl;
    output.close();
    std::cout << "Elapsed calculation time: " << elapsed_secs << " secs." << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 6) {
        std::cout << "Incorrect arguments" << std::endl;
        std::cout << "Format: ./program matrix_size threads_num file_name_1 file_name_2 matrix_type" << std::endl;
        return 1;
    }
    int matrix_size = atoi(argv[1]);
    int threads_num = atoi(argv[2]);
    std::string file_name_1 (argv[3]);
    std::string file_name_2 (argv[4]);
    std::string matrix_type (argv[5]);
    size_t array_size = pow(matrix_size, 2);
    std::cout << "Will be elapsed " << double((sizeof(int) * array_size * 3) / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "Using " << threads_num << " threads" << std::endl;
    if (matrix_type.compare("integer") == 0) {
        calculation<int>(array_size, file_name_1, file_name_2, threads_num);
    } else if (matrix_type.compare("double") == 0) {
        calculation<double>(array_size, file_name_1, file_name_2, threads_num);
    } else if (matrix_type.compare("vector") == 0) {
        calculation<int>(array_size, file_name_1, file_name_2, threads_num);
    } else {
        std::cout << "Unknown matrix type" << std::endl;
        return 1;
    }
}

// compiled with: g++ (GCC) 7.3.1 20180303 (Red Hat 7.3.1-5)
// scl enable devtoolset-7 bash
// g++ thread_main.cpp -o thread_main -pthread
