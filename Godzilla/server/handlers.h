#include <iostream>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>

#include "database.h"
#include "http_parser.h"

namespace pt = boost::property_tree;

class Handlers {
private:
    DatabaseService* db_service;

    http_raw_packet read_from_socket(int cs)
    {
        char buffer[2048+1];
        http_parser parser;

        int size = read(cs, buffer, 2048);

        std::cerr << "Received packet:\n";
        buffer[size] = '\0';
        std::cerr << buffer << std::endl;

        auto pack = parser.parse(std::string(buffer,strlen(buffer)));
        return pack;
    }

    void write_to_socket(int sock,http_raw_packet response) {
        http_parser p;
        write(sock,p.form(response).c_str(),p.form(response).size());
        std::cerr << "Send packet:\n" << p.form(response);
    }

    http_raw_packet generate_response(RFC2616::responses response) {
        http_raw_packet resp;
        static std::map<RFC2616::responses,std::string>response_map{
            std::make_pair(RFC2616::OK,"OK"),
            std::make_pair(RFC2616::NOT_FOUND,"Not Found")
        };
        resp.start = "HTTP/1.1 " + boost::lexical_cast<std::string>(response) + " " + response_map[response];
        return resp;
    }

    void send_result(int sock, pt::ptree json_data) {
        http_raw_packet response = generate_response(RFC2616::OK);
        http_field field1, field2;
        json_data.put("ok", 1);


        std::stringstream ss;
        pt::write_json(ss, json_data);
        std::string json_data_string = ss.str();
        std::cout << json_data_string << std::endl;

        field1.value = "application/json";
        field2.value = boost::lexical_cast<std::string>(json_data_string.length());
        field1.params.insert(std::make_pair("charset","utf-8"));
        response.body.insert(std::make_pair("Content-Type", field1));
        response.body.insert(std::make_pair("Content-Length", field2));
        response.content = json_data_string;
        write_to_socket(sock, response);
        return;
    }

    void send_error(int sock, std::string error_text) {
        http_raw_packet response = generate_response(RFC2616::NOT_FOUND);
        http_field field1, field2;
        pt::ptree root;
        root.put("ok", 0);
        root.put("error_msg", error_text);

        std::stringstream ss;
        pt::write_json(ss, root);
        std::string json_data = ss.str();
        std::cout << json_data << std::endl;

        field1.value = "application/json";
        field2.value = boost::lexical_cast<std::string>(json_data.length());
        field1.params.insert(std::make_pair("charset","utf-8"));
        response.body.insert(std::make_pair("Content-Type", field1));
        response.body.insert(std::make_pair("Content-Length", field2));
        response.content = json_data;
        write_to_socket(sock, response);
        return;
    }

    void login(http_packet pack, int sock) {
        pt::ptree root;
        auto params = pack.get_query_params();
        std::string login = boost::lexical_cast<std::string>(params["login"]);
        std::string password = boost::lexical_cast<std::string>(params["password"]);
        try {
            int id = db_service->login_user(login, password);
            if (id > 0) {
                pt::ptree answer;
                answer.put("id", id);
                send_result(sock, answer);
            } else {
                send_error(sock, std::string("Cant find creds for user"));
            }
        } catch (std::string str) {
            send_error(sock, str);
        }
    }

    void logout(http_packet pack, int sock) {
        pt::ptree root;
        auto params = pack.get_query_params();
        int cookie = boost::lexical_cast<int>(params["id"]);
        try {
            db_service->logout_user(cookie);
            pt::ptree answer;
            send_result(sock, answer);
        } catch (std::string str) {
            send_error(sock, str);
        }
    }

    void registration(http_packet pack, int sock) {
        pt::ptree root;
        std::stringstream ss;
        ss << pack.get_content();
        pt::read_json(ss, root);
        std::string login(root.get<std::string>("login"));
        std::string password(root.get<std::string>("password"));
        try {
            db_service->register_user(login, password);
            pt::ptree answer;
            send_result(sock, answer);
        } catch (std::string str) {
            send_error(sock, str);
        }
    }

    void get_contacts(http_packet pack, int sock) {
        pt::ptree root;
        pt::ptree children;
        std::vector<pt::ptree> childs_objects;

        int cookie = boost::lexical_cast<int>(pack.get_query_params()["id"]);
        std::string name_to_find = boost::lexical_cast<std::string>(pack.get_query_params()["name"]);
        std::string email_to_find = boost::lexical_cast<std::string>(pack.get_query_params()["email"]);
        std::string phone_to_find = boost::lexical_cast<std::string>(pack.get_query_params()["phone"]);
        std::vector<std::tuple<int, std::string, std::string, std::string>> contacts;

        if (!name_to_find.empty()) {
            name_to_find.insert(0, "%");
            name_to_find.append("%");
            contacts = db_service->find_contacts_by_name(cookie, name_to_find);
        } else if (!email_to_find.empty()) {
            email_to_find.insert(0, "%");
            email_to_find.append("%");
            contacts = db_service->find_contacts_by_email(cookie, email_to_find);
        } else if (!phone_to_find.empty()) {
            phone_to_find.insert(0, "%");
            phone_to_find.append("%");
            contacts = db_service->find_contacts_by_phone(cookie, phone_to_find);
        } else {
            contacts = db_service->get_contacts(cookie);
        }

        for (auto elem: contacts) {
            pt::ptree buf;
            buf.put("id", boost::lexical_cast<int>(std::get<0>(elem)));
            buf.put("name", std::get<1>(elem));
            buf.put("email", std::get<2>(elem));
            buf.put("phone_number", std::get<3>(elem));
            childs_objects.push_back(buf);
        }

        for (auto &it: childs_objects) {
            children.push_back(std::make_pair("", it));
        }

        root.add_child("contacts", children);
        send_result(sock, root);
    }

    void add_contact(http_packet pack, int sock) {
        pt::ptree root;
        std::stringstream ss;
        ss << pack.get_content();
        pt::read_json(ss, root);

        int cookie = boost::lexical_cast<int>(pack.get_query_params()["id"]);
        std::string name(root.get<std::string>("name"));
        std::string email(root.get<std::string>("email"));
        std::string phone_number(root.get<std::string>("phone_number"));

        try {
            db_service->add_contact(cookie, name, email, phone_number);
            pt::ptree answer;
            send_result(sock, answer);
        } catch (std::string str) {
            send_error(sock, str);
        }
    }

    void update_contact(http_packet pack, int sock) {
        pt::ptree root;
        std::stringstream ss;
        ss << pack.get_content();
        pt::read_json(ss, root);

        int cookie = boost::lexical_cast<int>(pack.get_query_params()["id"]);
        int record_id = root.get<int>("record_id");
        std::string name(root.get<std::string>("name"));
        std::string email(root.get<std::string>("email"));
        std::string phone_number(root.get<std::string>("phone_number"));

        try {
            db_service->update_contact(cookie, record_id, name, email, phone_number);
            pt::ptree answer;
            send_result(sock, answer);
        } catch (std::string str) {
            send_error(sock, str);
        }
    }

    void remove_contact(http_packet pack, int sock) {
        pt::ptree root;
        std::stringstream ss;
        ss << pack.get_content();
        pt::read_json(ss, root);

        int cookie = boost::lexical_cast<int>(pack.get_query_params()["id"]);
        int record_id = root.get<int>("record_id");

        try {
            db_service->remove_contact(cookie, record_id);
            pt::ptree answer;
            send_result(sock, answer);
        } catch (std::string str) {
            send_error(sock, str);
        }
    }

public:
    Handlers() {
        db_service = new DatabaseService("contacts.db");
    }

    ~Handlers() {
        delete db_service;
    }

    void handle_request(int sock) {
        auto pack = read_from_socket(sock);
        http_packet packet(&pack);
        auto line = packet.get_start().get<request_line>();
        auto uri = packet.get_route();
        auto method = line.request;

        try {
            if (uri == "/contacts") {
                if(method == "GET") {
                    // get users
                    std::cout << "Get on contacts" << std::endl;
                    get_contacts(packet, sock);
                } else if (method == "POST") {
                    // add new user
                    std::cout << "Post on contacts" << std::endl;
                    add_contact(packet, sock);
                } else if (method == "PUT") {
                    // update user info
                    std::cout << "Update on contacts" << std::endl;
                    update_contact(packet, sock);
                } else if (method == "DELETE") {
                    // delete user
                    std::cout << "Delete on contacts" << std::endl;
                    remove_contact(packet, sock);
                } else {
                    std::cerr << "Unknown method: " << method << " for " << uri << std::endl;
                    send_error(sock, std::string("Unknown method: " + method + " for " + uri));
                }
            } else if (uri == "/auth") {
                if(method == "GET") {
                    // log in
                    std::cout << "Get on auth" << std::endl;
                    login(packet, sock);
                } else if (method == "POST") {
                    // register
                    std::cout << "Post on auth" << std::endl;
                    registration(packet, sock);
                } else if (method == "DELETE") {
                    // logout
                    std::cout << "Delete on auth" << std::endl;
                    logout(packet, sock);
                } else {
                    std::cerr << "Unknown method: " << method << " for " << uri << std::endl;
                    send_error(sock, std::string("Unknown method: " + method + " for " + uri));
                }
            } else {
                std::cerr << "Unknown route: " << uri << std::endl;
                send_error(sock, std::string("Unknown route: " + uri));
            }
        } catch (...) {
            std::string error(boost::current_exception_diagnostic_information());
            std::cerr << error << std::endl;
            send_error(sock, error);
        }
        close(sock);
    }
};
