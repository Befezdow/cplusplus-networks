#include <omp.h>
#include <iostream>
#include <ctime>
#include <fstream>
#include <cmath>

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
void calculation(size_t array_size,
                std::string first_matrix_filename,
                std::string second_matrix_filename) {
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
    #pragma omp parallel for shared(result_matrix, matrix_1, matrix_2)
    for (i = 0; i <= array_size; i++) {
        result_matrix[i] = matrix_1[i] + matrix_2[i];
    }
    clock_t end = clock();
    std::cout << "Calculation finished " << std::endl;
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Starting record result matrix to result.txt" << std::endl;
    std::ofstream output;
    output.open("result.txt");
    for (int j = 0; j < array_size; j++) {
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
    omp_set_num_threads(threads_num);
    if (matrix_type.compare("integer") == 0) {
        calculation<int>(array_size, file_name_1, file_name_2);
    } else if (matrix_type.compare("double") == 0) {
        calculation<double>(array_size, file_name_1, file_name_2);
    } else if (matrix_type.compare("vector") == 0) {
        calculation<int>(array_size, file_name_1, file_name_2);
    } else {
        std::cout << "Unknown matrix type" << std::endl;
        return 1;
    }
}

// compiled with: g++ (GCC) 7.3.1 20180303 (Red Hat 7.3.1-5)
// scl enable devtoolset-7 bash
// g++ omp_main.cpp -o omp_main -fopenmp
