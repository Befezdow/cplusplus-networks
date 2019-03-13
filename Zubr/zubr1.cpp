#include <iostream>
#include <boost/program_options.hpp>
#include <fstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace po=boost::program_options;

po::variables_map get_command_line_arguments(int argc, char** argv) {
    po::options_description desc("General options");
    desc.add_options()
            ("help,h", "Show help")
            ("count,c", po::value<int>()->required(), "Count of rows in matrix dimension file")
            ;

    po::options_description files_desc("File options");
    files_desc.add_options()
            ("input,i", po::value<std::string>()->required(), "Name of file with matrix dimension (required)")
            ("output,o", po::value<std::string>()->required(), "Prefix of files with matrixes (required)")
            ;

    desc.add(files_desc);
    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
    po::store(parsed, vm);

    if (!vm["help"].empty()) {
        std::cout << desc << std::endl;
        exit(0);
    }

    return vm;
}

std::vector<int> generate_matrix_data(int width, int height) {
    std::vector<int> result;

    result.push_back(width);
    result.push_back(height);
    for (int i = 0; i < width * height; ++i) {
        result.push_back(std::rand() % 100);
    }

    return result;
}

void generate_file_with_dimensions(std::string filename, int rows_count = 100, int max_dimension_size = 5) {
    std::ofstream file;
    file.open(filename);
    if (!file) {
        std::cerr << "generate_file_with_dimensions :: Unable to open file";
        exit(1);
    }

    for (int i = 0; i < rows_count; ++i) {
        file << (std::rand() % max_dimension_size) + 1;

        if (i != rows_count - 1) {
            file << " ";
        }
    }

    file.close();
}

void generate_matrix_files(std::string dimensions_filename, std::string files_prefix, std::string files_extension, int stream_descriptor) {
    std::ifstream in_file;
    in_file.open(dimensions_filename);
    if (!in_file) {
        std::cerr << "generate_matrix_files :: Unable to open input file";
        exit(1);
    }

    int count = 0;
    while(in_file) {
        int dimension;
        in_file >> dimension;
        if (!in_file) {
            break;
        }

        std::string output_filename = files_prefix + std::to_string(count++) + "." + files_extension;
        std::ofstream of_file;
        of_file.open(output_filename);
        if (!of_file) {
            std::cerr << "generate_matrix_files :: Unable to open output file";
            exit(1);
        }

        std::vector<int> matrix_data = generate_matrix_data(dimension, dimension);
        int vector_size = matrix_data.size();
        for (int i = 0; i < vector_size; ++i) {
            of_file << matrix_data[i];

            if (i != vector_size - 1) {
                of_file << " ";
            }
        }

        of_file.close();

        size_t size = output_filename.length();
        write(stream_descriptor, &size, sizeof(size_t));
        write(stream_descriptor, output_filename.c_str(), output_filename.length());
    }

    size_t size = 0;    //mark stream as ended
    write(stream_descriptor, &size, sizeof(size_t));

    in_file.close();
}

void handle_input_fifo(int descriptor, std::string out_filename) {
    std::ofstream file;
    file.open(out_filename);
    if (!file) {
        std::cerr << "handle_in_fifo :: Unable to open file";
        exit(1);
    }

    while(1) {
        float determinant;
        float t = read(descriptor, &determinant, sizeof(float));
        
        if (t == -1) {
            std::cerr << "Unable to read from FIFO" << std::endl;
            exit(1); 
        }
        if (t == 0) {
            break;
        }

        file << determinant << std::endl;
    }

    file.close();
}

int main(int argc, char** argv) {
    
    po::variables_map vm = get_command_line_arguments(argc, argv);

    std::string matrix_files_extension = "txt";
    std::string in_stream_name = "/tmp/fifo0002.2";
    std::string out_stream_name = "/tmp/fifo0001.1";

    if (vm["input"].empty() || vm["output"].empty() || vm["count"].empty()) {
            std::cerr << "One of required parameters is empty" << std::endl;
            return 0;
    }

    std::string dimension_file = vm["input"].as<std::string>();
    std::string matrix_files_prefix = vm["output"].as<std::string>();
    int rows_count = vm["count"].as<int>();

    // std::string dimension_file = "/home/befezdow/Workspace/study/Lobovikov/Befezdow/Zubr/dimensions.txt";
    // std::string matrix_files_prefix = "/home/befezdow/Workspace/study/Lobovikov/Befezdow/Zubr/matrixes/matrix";
    
    generate_file_with_dimensions(dimension_file);

    unlink(out_stream_name.c_str());
    if (mkfifo(out_stream_name.c_str(), 0777) == -1) {
        std::cerr << "Unable to create FIFO" << std::endl;
        exit(1);
    }
    int out_descriptor = open(out_stream_name.c_str(), O_WRONLY);
    if (out_descriptor == -1) {
        std::cerr << "Unable to open out FIFO" << std::endl;
        exit(1);
    }

    int in_descriptor = open(in_stream_name.c_str(), O_RDONLY);
    if (in_descriptor == -1) {
        std::cerr << "Unable to open in FIFO" << std::endl;
        exit(1);
    }

    generate_matrix_files(dimension_file, matrix_files_prefix, matrix_files_extension, out_descriptor);

    handle_input_fifo(in_descriptor, "/home/befezdow/Workspace/study/Lobovikov/Befezdow/Zubr/determinants.txt");

    close(out_descriptor);
    close(in_descriptor);
    unlink(out_stream_name.c_str());

    return 0;  
}

//c++ zubr1.cpp -o zubr -lboost_program_options -lboost_system