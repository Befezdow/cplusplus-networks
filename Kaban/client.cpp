#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctime>
#define seconds_from_1900_to_1970 2244988787

int main(int argc, char ** argv) {
	if (argc < 2) {
		std::cerr << "Incorrect count of parameters" << std::endl;
		return 1;
	}

	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd < 0) {
		std::cerr << "Create socket error: " << errno << std::endl;
		return 1;
	}

	struct sockaddr_in server_info; 
 	server_info.sin_family = AF_INET;
 	server_info.sin_port = htons(37);
 	server_info.sin_addr.s_addr = inet_addr(argv[1]);

 	socklen_t server_info_len = sizeof(server_info);

 	char symbol = '\n';
 	int res = sendto(sockfd, &symbol, sizeof(char), 0, (struct sockaddr *)&server_info, server_info_len);
 	if (res < 0) {
 		std::cerr << "Send error: " << errno << std::endl;
		return 1;
 	}

 	long seconds_count;
 	res = recvfrom(sockfd, &seconds_count, sizeof(long), 0, (struct sockaddr *)&server_info, &server_info_len);
 	if (res < 0) {
 		std::cerr << "Receive error: " << errno << std::endl;
		return 1;
 	}

 	long normal_seconds = ntohl(seconds_count);
 	long normal_seconds_from_1970 = normal_seconds - seconds_from_1900_to_1970;
 	std::time_t t = normal_seconds_from_1970;

 	std::cout << "Seconds: " << normal_seconds_from_1970 << " Date: " << ctime(&t) << std::endl;
}