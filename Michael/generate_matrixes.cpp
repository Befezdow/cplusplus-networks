#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <time.h>
#include "vector.h"

int main (int argc, char *argv[]) {
    if (argc < 4) {
        std::cout << "Incorrect arguments" << std::endl;
        std::cout << "Format: ./program matrix_type matrix_size output_file_name" << std::endl;
        return 1;
    }
    std::string arg(argv[1]);
    int matrix_size = atoi(argv[2]);
    std::string file_name(argv[3]);

    srand (time(NULL));

    std::ofstream file;
    file.open (file_name + ".txt", std::ios::binary);

    if (arg.compare("integer") == 0) {
        for (int i = 0; i < pow(matrix_size, 2); ++i) {
            int number = std::rand() % 1000;
            file << number << ' ';
        }
    } else if (arg.compare("double") == 0) {
        for (int i = 0; i < pow(matrix_size, 2); ++i) {
            double number = M_PI / (std::rand() % 1000);
            file << number << ' ';
        }
    } else if (arg.compare("vector") == 0) {
        for (int i = 0; i < pow(matrix_size, 2); ++i) {
            Vector vector(std::rand() % 1000, std::rand() % 1000, std::rand() % 1000);
            file << vector;
        }
    } else {
        std::cout << "Unknown matrix type" << std::endl;
        return 1;
    }

    file.close();
}

// compiled with: g++ (GCC) 7.3.1 20180303 (Red Hat 7.3.1-5)
// scl enable devtoolset-7 bash
// g++ generate_matrixes.cpp -o generate_matrixes
