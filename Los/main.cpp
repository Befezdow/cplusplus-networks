#include <iostream>
#include <fstream>
#include <sys/wait.h>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <cstring>
#include <sys/mman.h>
#include <memory.h>
#include "calculator/calc.h"

#define MEMORY_SIZE 10
#define OUTPUT_FILE_NAME "/home/befezdow/Workspace/study/Lobovikov/Befezdow/Los/output.txt"

#define QUEUE_NAME "/los_queue"
#define QUEUE_MAX_MESSAGES_COUNT 10
#define QUEUE_MAX_MESSAGE_SIZE 256

#define SHARED_MEMORY_NAME "/los_shared_memory"

enum BlockStatus {init = 0, ready_for_calculation = 1, calculated = 2};

#define SHARED_MEMORY_REQUEST_OFFSET 0
#define SHARED_MEMORY_RESPONSE_OFFSET MEMORY_SIZE * QUEUE_MAX_MESSAGE_SIZE
#define SHARED_MEMORY_RESULT_STATUS_OFFSET SHARED_MEMORY_RESPONSE_OFFSET + MEMORY_SIZE * sizeof(double)
#define SHARED_MEMORY_END_OFFSET SHARED_MEMORY_RESULT_STATUS_OFFSET + MEMORY_SIZE * sizeof(char)

std::string generate_expression(int length) {
    const std::string dict = "+-*/";
    const int max = 999;

    std::string result = "";
    for (int i = 0; i < length; ++i) {
        result += std::to_string(std::rand() % max);
        if (i < length - 1) {
            result += " ";
            result += dict[std::rand() % dict.length()];
            result += " ";
        }
    }

    return result;
}

void write_to_queue(mqd_t desc, std::string message) {
    int status = mq_send(desc, message.c_str(), QUEUE_MAX_MESSAGE_SIZE, 1);
    if (status == -1) {
        std::cerr << "write_to_queue : Can't write to queue. Error: " << std::strerror(errno) << std::endl;
    }
}

std::string read_from_queue(mqd_t desc) {
    char* buffer = new char[QUEUE_MAX_MESSAGE_SIZE];
    int status = mq_receive(desc, buffer, QUEUE_MAX_MESSAGE_SIZE, NULL);
    if (status == -1) {
        std::cerr << "read_from_queue : Can't read from queue. Error: " << std::strerror(errno) << std::endl;
    }

    std::string result(buffer);
    delete [] buffer;
    return result;
}

void write_result(std::string request, double response, std::ofstream& file) {
    file << request << " = " << response << std::endl;
}

void zero_process() {
    mqd_t desc = mq_open(QUEUE_NAME, O_WRONLY);
    if (desc == -1) {
        std::cerr << "zero_process : Unable to open queue. Error: " << std::strerror(errno) << std::endl;
        exit(1);
    }

    while (true) {
    	write_to_queue(desc, generate_expression((std::rand() % 4) + 2));
        sleep(1);
    }

    // mq_close(desc);
}

void first_process() {
    mqd_t queue_desc = mq_open(QUEUE_NAME, O_RDONLY);
    if (queue_desc == -1) {
        std::cerr << "first_process : Unable to open queue. Error: " << std::strerror(errno) << std::endl;
        exit(1);
    }

    int memory_desc = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0777);
    if (memory_desc == -1) {
        std::cerr << "first_process : Unable to open shared memory. Error: " << std::strerror(errno) << std::endl;
        mq_close(queue_desc);
        exit(1);
    }

    void* memory = mmap(NULL, SHARED_MEMORY_END_OFFSET, PROT_READ | PROT_WRITE, MAP_SHARED, memory_desc, 0);
    if (memory == MAP_FAILED) {
        std::cerr << "first_process : Unable to mmap shared memory. Error: " << std::strerror(errno) << std::endl;
    }

    while (true) {
    	std::string message = read_from_queue(queue_desc);
    	int i = 0;
    	while (true) {
    		bool exit_flag = false;
    		while (true) {
    			char status;
	    		std::memcpy(&status, memory + SHARED_MEMORY_RESULT_STATUS_OFFSET + i * sizeof(char), sizeof(char));
	    		if (status == (char) BlockStatus::init) {
	    			exit_flag = true;
	    			break;
	    		}

	    		i++;
	    		if (i >= MEMORY_SIZE) {
	    			i = 0;
	    		}
    		}

    		if (exit_flag) {
    			break;
    		}
    	}

		char status = (char) BlockStatus::ready_for_calculation;
    	std::memcpy(memory + SHARED_MEMORY_REQUEST_OFFSET + i * QUEUE_MAX_MESSAGE_SIZE, message.c_str(), QUEUE_MAX_MESSAGE_SIZE);
    	std::memcpy(memory + SHARED_MEMORY_RESULT_STATUS_OFFSET + i * sizeof(char), &status, sizeof(char));
    }

    // if (munmap(NULL, SHARED_MEMORY_RESPONSE_OFFSET) == -1) {
    //     std::cerr << "first_process : Unable to munmap shared memory. Error: " << std::strerror(errno) << std::endl;
    // }

    // mq_close(queue_desc);
}

void second_process() {
    int memory_desc = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0777);
    if (memory_desc == -1) {
        std::cerr << "second_process : Unable to open shared memory. Error: " << std::strerror(errno) << std::endl;
        exit(1);
    }

    void* memory = mmap(NULL, SHARED_MEMORY_END_OFFSET, PROT_READ | PROT_WRITE, MAP_SHARED, memory_desc, 0);
    if (memory == MAP_FAILED) {
        std::cerr << "second_process : Unable to mmap shared memory. Error: " << std::strerror(errno) << std::endl;
    }

    while (true) {
    	int i = 0;
    	while (true) {
    		bool exit_flag = false;
    		while (true) {
    			char status;
	    		std::memcpy(&status, memory + SHARED_MEMORY_RESULT_STATUS_OFFSET + i * sizeof(char), sizeof(char));
	    		if (status == (char) BlockStatus::ready_for_calculation) {
	    			exit_flag = true;
	    			break;
	    		}

	    		i++;
	    		if (i >= MEMORY_SIZE) {
	    			i = 0;
	    		}
    		}

    		if (exit_flag) {
    			break;
    		}
    	}

    	char* buffer = new char[QUEUE_MAX_MESSAGE_SIZE];
        std::memcpy(buffer, memory + SHARED_MEMORY_REQUEST_OFFSET + i * QUEUE_MAX_MESSAGE_SIZE, QUEUE_MAX_MESSAGE_SIZE);
        std::string temp(buffer);
        delete [] buffer;

        int error = 0;
        double calculated_result = calc(temp, &error);
        if (error != 0) {
            std::cerr << "second_process : Calculate error achieved" << std::endl;
        }

        char status = (char) BlockStatus::calculated;
        std::memcpy(memory + SHARED_MEMORY_RESPONSE_OFFSET + i * sizeof(double), &calculated_result, sizeof(double));
        std::memcpy(memory + SHARED_MEMORY_RESULT_STATUS_OFFSET + i * sizeof(char), &status, sizeof(char));
    }

    // if (munmap(NULL, SHARED_MEMORY_RESULT_STATUS_OFFSET) == -1) {
    //     std::cerr << "second_process : Unable to munmap shared memory. Error: " << std::strerror(errno) << std::endl;
    // }
}

void third_process() {
    int memory_desc = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0777);
    if (memory_desc == -1) {
        std::cerr << "third_process : Unable to open shared memory. Error: " << std::strerror(errno) << std::endl;
        exit(1);
    }

    void* memory = mmap(NULL, SHARED_MEMORY_END_OFFSET, PROT_READ | PROT_WRITE, MAP_SHARED, memory_desc, 0);
    if (memory == MAP_FAILED) {
        std::cerr << "third_process : Unable to mmap shared memory. Error: " << std::strerror(errno) << std::endl;
    }

    std::ofstream file;
    file.open(OUTPUT_FILE_NAME);
    if (!file) {
        std::cerr << "third_process :: Unable to open output file";
        exit(1);
    }

    while (true) {
    	int i = 0;
    	while (true) {
    		bool exit_flag = false;
    		while (true) {
    			char status;
	    		std::memcpy(&status, memory + SHARED_MEMORY_RESULT_STATUS_OFFSET + i * sizeof(char), sizeof(char));
	    		if (status == (char) BlockStatus::calculated) {
	    			exit_flag = true;
	    			break;
	    		}

	    		i++;
	    		if (i >= MEMORY_SIZE) {
	    			i = 0;
	    		}
    		}

    		if (exit_flag) {
    			break;
    		}
    	}

    	char* buffer = new char[QUEUE_MAX_MESSAGE_SIZE];
        std::memcpy(buffer, memory + SHARED_MEMORY_REQUEST_OFFSET + i * QUEUE_MAX_MESSAGE_SIZE, QUEUE_MAX_MESSAGE_SIZE);
        std::string request(buffer);

        double response;
        std::memcpy(&response, memory + SHARED_MEMORY_RESPONSE_OFFSET + i * sizeof(double), sizeof(double));

        std::cout << request << " = " << response << std::endl;
        write_result(request, response, file);

        char status = (char) BlockStatus::init;
        std::memcpy(memory + SHARED_MEMORY_RESULT_STATUS_OFFSET + i * sizeof(char), &status, sizeof(char));
    }

    // file.close();

    // if (munmap(NULL, SHARED_MEMORY_END_OFFSET) == -1) {
    //     std::cerr << "third_process : Unable to munmap shared memory. Error: " << std::strerror(errno) << std::endl;
    // }
}

int main() {
    if(mq_unlink(QUEUE_NAME) == 0) {
        std::cout << "Message queue '" << QUEUE_NAME << "' removed from the system" << std::endl;
    }

    if (shm_unlink(SHARED_MEMORY_NAME) == 0) {
        std::cout << "Shared memory '" << SHARED_MEMORY_NAME <<"' removed from the system" << std::endl;
    }

    struct mq_attr queue_config;
    queue_config.mq_flags = 0;
    queue_config.mq_maxmsg = QUEUE_MAX_MESSAGES_COUNT;
    queue_config.mq_msgsize = QUEUE_MAX_MESSAGE_SIZE;
    queue_config.mq_curmsgs = 0;
    mqd_t desc = mq_open(QUEUE_NAME, O_WRONLY | O_CREAT, 0777, &queue_config);
    if (desc == -1) {
        std::cerr << "main : Unable to create queue. Error: " << std::strerror(errno) << std::endl;
        return 1;
    }
    mq_close(desc);

    int memory_desc = shm_open(SHARED_MEMORY_NAME, O_RDWR | O_CREAT, 0777);
    if (memory_desc == -1) {
        std::cerr << "main : Unable to create shared memory. Error: " << std::strerror(errno) << std::endl;
        return 1;
    }

    if (ftruncate(memory_desc, SHARED_MEMORY_RESULT_STATUS_OFFSET + MEMORY_SIZE * sizeof(bool)) == -1) {
        std::cerr << "main : Unable to truncate shared memory. Error: " << std::strerror(errno) << std::endl;
        return 1;
    }

    void* memory = mmap(NULL, SHARED_MEMORY_END_OFFSET, PROT_WRITE, MAP_SHARED, memory_desc, 0);
    if (memory == MAP_FAILED) {
        std::cerr << "main : Unable to mmap shared memory. Error: " << std::strerror(errno) << std::endl;
    }

    char init_value = (char) BlockStatus::init;
    for (int i = 0; i < MEMORY_SIZE; ++i) {
        std::memcpy(memory + SHARED_MEMORY_RESULT_STATUS_OFFSET + i * sizeof(char), &init_value, sizeof(char));
    }

    if (munmap(NULL, SHARED_MEMORY_END_OFFSET) == -1) {
        std::cerr << "main : Unable to munmap shared memory. Error: " << std::strerror(errno) << std::endl;
    }

    for(int i = 0; i < 4; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            switch (i) {
                case 0:
                    zero_process();
                    break;
                case 1:
                    first_process();
                    break;
                case 2:
                    second_process();
                    break;
                case 3:
                    third_process();
                    break;
            }
            exit(0);
        }
    }

    wait(NULL);
}

// Build: g++ -std=c++17 main.cpp  calculator/*.cpp -o main -w -lrt