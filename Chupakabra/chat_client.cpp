#include "chat.h"
#include <iostream>
#include <string.h>

int
login(CLIENT * client) {
	login_result  *result;
	login_params  login_args;
	std::string login;
	std::string password;
	std::cout << "Enter your login: ";
	std::cin >> login;
	std::cout << "Enter your password: ";
	std::cin >> password;
	// std::cout << "Login: " << login << " Password: " << password << std::endl;
	login_args.login = (char*)login.c_str();
	login_args.password = (char*)password.c_str();
	result = login_2(&login_args, client);
	if (result == (login_result *) NULL) {
		clnt_perror (client, "Login failed");
		return 0;
	}
	if (result->res.code == result_code::FAIL) {
		std::cout << "Login failed. Reason: " << std::string(result->res.descr) << std::endl;
		return 0;
	}
	return result->cookie;
}

bool
registration(CLIENT * client) {
	result  *reg_result;
	register_params  register_args;
	std::string login;
	std::string password;
	std::cout << "Enter your login: ";
	std::cin >> login;
	std::cout << "Enter your password: ";
	std::cin >> password;
	// std::cout << "Login: " << login << " Password: " << password << std::endl;
	register_args.login = (char*)login.c_str();
	register_args.password = (char*)password.c_str();
	reg_result = register_2(&register_args, client);
	if (reg_result == (result *) NULL) {
		clnt_perror (client, "Register failed");
		return false;
	}
	if (reg_result->code == result_code::FAIL) {
		std::cout << "Register failed. Reason " << reg_result->descr << std::endl;
		return false;
	}
	return true;
}

bool
list_of_users(CLIENT * client, int cookie) {
	users_result  *users_list_result;
	users_param  users_args;
	// std::cout << "List of users: " << cookie << std::endl;
	users_args.cookie = cookie;
	users_list_result = users_2(&users_args, client);
	if (users_list_result == (users_result *) NULL) {
		clnt_perror (client, "List of users failed");
		return false;
	}
	if (users_list_result->res.code == result_code::FAIL) {
		std::cout << "List of users failed. Reason: " << users_list_result->res.descr << std::endl;
		return false;
	}
	users_message * users = users_list_result->data.data_val;
	for (int i = 0; i < users_list_result->data.data_len; i++) {
		std::string user_status = users[i].online ? "Online" : "Offline";
		std::cout << i << ") Login: " << users[i].login << " | Status: " << user_status << std::endl;
	}
	return true;
}

bool
send_msg(CLIENT * client, int cookie) {
	result  * send_result;
	send_params  send_args;
	std::string target_login;
	std::string message;
	std::cout << "Enter target login: ";
	std::cin >> target_login;
	std::cout << "Enter your message: ";
	std::cin.ignore();
	std::cin.clear();
	std::getline(std::cin, message);
	// std::cout << "Target login: " << target_login << " Message: " << message << std::endl;
	send_args.to = (char*)target_login.c_str();
	send_args.cookie = cookie;
	send_args.message = (char*)message.c_str();
	send_result = send_2(&send_args, client);
	if (send_result == (result *) NULL) {
		clnt_perror (client, "Send failed");
		return false;
	}
	if (send_result->code == result_code::FAIL) {
		std::cout << "Send failed. Reason: " << send_result->descr << std::endl;
		return false;
	}
	return true;
}

bool
receive_msg(CLIENT * client, int cookie) {
	receive_result  * result;
	receive_params  recv_args;
	// std::cout << "Receive: " << cookie << std::endl;
	recv_args.cookie = cookie;
	result = receive_2(&recv_args, client);
	if (result == (receive_result *) NULL) {
		clnt_perror (client, "Receive failed");
		return false;
	}
	if (result->res.code == result_code::FAIL) {
		std::cout << "Receive failed. Reason: " << result->res.descr << std::endl;
		return false;
	}
	receive_message * messages = result->data.data_val;
	for (int i = 0; i < result->data.data_len; i++) {
		std::cout << i << ") From: " << messages[i].from << " | Message: " << messages[i].message << std::endl;
	}
	return true;
}

bool
logout(CLIENT * client, int cookie) {
	result  *logout_result;
	logout_params  logout_args;
	// std::cout << "Logout: " << cookie << std::endl;
	logout_args.cookie = cookie;
	logout_result = logout_2(&logout_args, client);
	if (logout_result == (result *) NULL) {
		clnt_perror (client, "Logout failed");
		return false;
	}
	if (logout_result->code == result_code::FAIL) {
		std::cout << "Logout failed. Reason: " << logout_result->descr << std::endl;
		return false;
	}
	return true;
}

void
main_loop(char* host) {
	CLIENT * client = clnt_create (host, RPC_CHAT, RPC_CHAT_VERSION_2, "udp");
	int my_cookie = 0;
	if (client == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
	std::cout << "Client started" << std::endl;
	bool is_active = true;
	while (is_active) {
		int option = 0;
		if (my_cookie == 0) {
			std::cout << "-----------------------------------------------------------" << std::endl;
			std::cout << "Enter string number to select one of options: " << std::endl
				<< "1) Login" << std::endl
				<< "2) Register" << std::endl
				<< "3) Exit" << std::endl
				<< "Enter option: ";
			std::cin >> option;
			switch(option) {
				case 1:
					// std::cout << "Login" << std::endl;
					my_cookie = login(client);
					if (my_cookie) {
						std::cout << "Succesful login. My cookie is: " << my_cookie << std::endl;
					}
					break;
				case 2:
					// std::cout << "Register" << std::endl;
					if (registration(client)) {
						std::cout << "Succesful registration" << std::endl;
					} else {
						std::cout << "Registration failed" << std::endl;
					}
					break;
				case 3:
					std::cout << "Goodbye" << std::endl;
					is_active = false;
					break;
				default:
					std::cout << "Unknown option " << option << std::endl;
					option = 0;
					std::cin.clear();
					std::cin.ignore(10000,'\n');
					break;
			}
		} else {
			std::cout << "-----------------------------------------------------------" << std::endl;
			std::cout << "Enter string number to select one of options: " << std::endl
				<< "1) List of users" << std::endl
				<< "2) Send message" << std::endl
				<< "3) Show messages" << std::endl
				<< "4) Logout" << std::endl
				<< "5) Exit" << std::endl
				<< "Enter option: ";
			std::cin >> option;
			// std::cout << "Selected option is " << option << std::endl;
			switch(option) {
				case 1:
					// std::cout << "List of users" << std::endl;
					if (!list_of_users(client, my_cookie)) {
						std::cout << "Can't show list of users" << std::endl;
					}
					break;
				case 2:
					// std::cout << "Send message" << std::endl;
					if (send_msg(client, my_cookie)) {
						std::cout << "Message succesful send" << std::endl;
					} else {
						std::cout << "Message send failed" << std::endl;
					}
					break;
				case 3:
					// std::cout << "Show messages" << std::endl;
					if (!receive_msg(client, my_cookie)) {
						std::cout << "Can't show messages" << std::endl;
					}
					break;
				case 4:
					if (logout(client, my_cookie)) {
						std::cout << "Succesful logout" << std::endl;
						my_cookie = 0;
					} else {
						std::cout << "Logout failed" << std::endl;
					}
					break;
				case 5:
					std::cout << "Goodbye" << std::endl;
					is_active = false;
					break;
				default:
					std::cout << "Unknown option " << option << std::endl;
					option = 0;
					std::cin.clear();
					std::cin.ignore(10000,'\n');
					break;
			}
		}
	}
	clnt_destroy(client);
	return;
}

int
main (int argc, char *argv[])
{
	char *host;

	if (argc < 2) {
		printf ("usage: %s server_host\n", argv[0]);
		exit (1);
	}
	host = argv[1];
	main_loop(host);
	exit (0);
}

//g++ -std=c++11  chat_client.cpp chat_clnt.c chat_xdr.c -o chat_client -lnsl -lsqlite3 -w
