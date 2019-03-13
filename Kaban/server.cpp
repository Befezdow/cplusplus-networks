#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctime>
#define seconds_from_1900_to_1970 2244988787

int main(int argc, char ** argv) {
	int status;
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(NULL, "37", &hints, &server_info)) != 0) {
		std::cout << "Getaddrinfo error: " << status << std::endl;
	    return 1;
	}

	int sockfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	bind(sockfd, server_info->ai_addr, server_info->ai_addrlen);
	std::cout << "Waiting for requests" << std::endl;

	while (1) {
		char symbol;
		struct addrinfo client_info;
		socklen_t client_info_size = sizeof(client_info);
		if ((status = recvfrom(sockfd, &symbol, sizeof(char), 0, (struct sockaddr *)&client_info, &client_info_size)) < 0) {
			std::cout << "Recvfrom error: " << status << std::endl;
		    break;
		}
		if (symbol != '\n') {
			std::cout << "Incorrect symbol: " << symbol << std::endl;
			continue;
		}
		long current_seconds = seconds_from_1900_to_1970 + time(NULL);
		long result = htonl(current_seconds);
		sendto(sockfd, &result, sizeof(long), 0, (struct sockaddr *)&client_info, client_info_size);
	}
	close(sockfd);
}