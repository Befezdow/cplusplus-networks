#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
 
std::vector<std::vector<float>> read_matrix_from_file(std::string filename) {
    std::ifstream file;
    file.open(filename);
    if (!file) {
        std::cerr << "read_matrix_from_file :: Unable to open input file";
        exit(1);
    }

    std::vector<int> matrix_data;
    int width;
    int height;
    file >> width;
    file >> height;

    std::vector<std::vector<float>> result;

    for (int i = 0; i < height; ++i) {
        result.push_back(std::vector<float>());
        for (int j = 0; j < width; ++j) {
            float temp;
            file >> temp;
            result[i].push_back(temp);
        }
    }

    file.close();

    return result;
}

void transform_matrix_into_triangle(std::vector<std::vector<float>>& matrix) {
    int width = matrix[0].size();
    int height = matrix.size();

    for (int i = 1; i < width; ++i)
        for (int k = i; k < width; ++k)
            for (int j = width-1; j >= 0; --j)
                matrix[k][j] -= (matrix[k][i-1] / matrix[i-1][i-1] * matrix[i-1][j]);
}

std::pair<float, bool> get_triangle_matrix_determinant(const std::vector<std::vector<float>>& matrix) {
    int width = matrix[0].size();
    int height = matrix.size();

    if (width != height) {
        return std::make_pair(0, false);
    }

    float result = 1;
    for (int i = 0; i < width; ++i) {
        result *= matrix[i][i];
    }

    return std::make_pair(result, true);
}

void handle_input_fifo(int in_descriptor, int out_descriptor) {
	while(1) {
    	size_t str_size;
    	int t = read(in_descriptor, &str_size, sizeof(size_t));
    	if (t == -1) {
    		std::cerr << "Unable to read from FIFO" << std::endl;
    		exit(1); 
    	}
    	if (t == 0 || str_size == 0) {
    		break;
    	}

    	char str[str_size + 1];
    	t = read(in_descriptor, str, str_size);
    	std::string std_str(str, str_size);

    	std::vector<std::vector<float>> matrix = read_matrix_from_file(std_str);
    	transform_matrix_into_triangle(matrix);
    	std::pair<float, bool> determinant =  get_triangle_matrix_determinant(matrix);
    	if (!determinant.second) {
    		std::cerr << "Cant encount determinant of non square matrix" << std::endl;
    	}
    	write(out_descriptor, &determinant.first, sizeof(float));
    }
}

int main() {
	std::string in_stream_name = "/tmp/fifo0001.1";
	std::string out_stream_name = "/tmp/fifo0002.2";

	int in_descriptor = open(in_stream_name.c_str(), O_RDONLY);
    if (in_descriptor == -1) {
        std::cerr << "Unable to open in FIFO" << std::endl;
        exit(1);
    }

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

	handle_input_fifo(in_descriptor, out_descriptor);    

    close(in_descriptor);
    close(out_descriptor);
    unlink(out_stream_name.c_str());

    return 0;
}