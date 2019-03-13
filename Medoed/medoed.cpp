#include <iostream>
#include <fstream>
#include <boost/property_tree/json_parser.hpp>
#include <sys/wait.h>
#include <algorithm>
#include <ctime>

std::string generate_string(int len) {
    static const std::string dict = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::string result = "";

    for (int i = 0; i < len; ++i) {
        result += dict[std::rand() % (sizeof(dict) - 1)];
    }

    return result;
}

void generate_files(std::string prefix, int files_count, int rows_count, int words_per_row) {
	std::ofstream file;
	for (int i = 0; i < files_count; ++i) {
		file.open(prefix + std::to_string(i) + ".txt");
		if (!file) {
    		std::cerr << "generate_files :: Unable to open file";
    		exit(1);
		}

		for (int j = 0; j < rows_count; ++j) {
			for (int k = 0; k < words_per_row; ++k) {
				std::string str = generate_string((std::rand() % 10) + 1);
				if (k != words_per_row - 1) {
					file << str << " ";
				}
			}
			file << std::endl;
		}

		file.close();
	}
}

std::pair<int, int> get_file_info(std::string filename) {
	std::ifstream file;
	file.open(filename);
	if (!file) {
    	std::cerr << "get_file_info :: Unable to open file";
    	exit(1);
	}

	int number_of_rows = 0;
	int number_of_words = 0;
	while (file) {
		std::string line;
		std::getline(file, line);
		if (!file) {
			break;
		}
		number_of_rows++;
		number_of_words += std::count(line.begin(), line.end(), ' ') + 1;
	}

	file.close();

	return std::make_pair(number_of_rows, number_of_words);
}

void handle_file(std::string in_prefix, std::string out_prefix, std::string postfix) {
	clock_t begin = clock();
	std::pair<int, int> file_info = get_file_info(in_prefix + postfix);
	clock_t end = clock();

	std::ofstream file;
	file.open(out_prefix + postfix);
	if (!file) {
		std::cerr << "handle_file :: Unable to open file";
		exit(1);
	}

	file << "Name: " << in_prefix + postfix << std::endl;
	file << "Rows count: " << file_info.first << std::endl;
	file << "Words count: " << file_info.second << std::endl;
	file << "Pid: " << getpid() << std::endl;
	file << "Time: " << double(end - begin) / CLOCKS_PER_SEC << std::endl;

	file.close();
}

void wait_for_processes(std::vector<pid_t> pids) {
	for (int i = 0; i < pids.size(); ++i) {
		wait(&pids[i]);
	}
}

int main() {
	std::ifstream configFile;
	configFile.open("./medoed_conf.json");
	if (!configFile) {
    	std::cerr << "main :: Unable to open config file";
    	exit(1);
	}

	boost::property_tree::ptree ptree;
	boost::property_tree::read_json(configFile, ptree);

	configFile.close();

	std::string home_path(getenv("HOME"));
	std::string in_prefix = ptree.get("inPrefix", home_path + "/medoedIn");
	std::string out_prefix = ptree.get("outPrefix", home_path + "/medoedOut");
	int rows_count = ptree.get("rowsCount", 5);
	int words_per_row = ptree.get("wordsPerRow", 5);
	int processes_count = ptree.get("processesCount", 5);

	std::cout << "In prefix: " << in_prefix << std::endl;
	std::cout << "Out prefix: " << out_prefix << std::endl;
	std::cout << "Rows count per file: " << rows_count << std::endl;
	std::cout << "Words count per row: " << words_per_row << std::endl;
	std::cout << "Processes count: " << processes_count << std::endl;

	generate_files(in_prefix, processes_count, rows_count, words_per_row);
	std::vector<pid_t> pids;
	for(int i = 0; i < processes_count; ++i) {
		pid_t pid = fork();
		if (pid == 0) {
			handle_file(in_prefix, out_prefix, std::to_string(i) + ".txt");
			exit(0);
		} else {
			pids.push_back(pid);
		}
	}

	wait_for_processes(pids);
}
