#include <sqlite3.h>
#include <string>
#include <iostream>
#include <vector>
#include <tuple>

class DatabaseService {
private:
    sqlite3* db;

    std::string get_user_login(int cookie) {
        sqlite3_stmt *stmt;

        if (sqlite3_prepare(db, "SELECT login FROM users WHERE cookie = ?", -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "get_user_login :: Could not prepare statement" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_int(stmt, 1, cookie) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "get_user_login :: Could not bind login" << std::endl;
            throw std::string("Internal server error");
        }

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cout << "Login not found" << std::endl;
            throw std::string("Login not found");
        }
        if (rc != SQLITE_ROW) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "get_user_login :: Could not step (execute) stmt" << std::endl;
            throw std::string("Internal server error");
        }
        std::string result((char*)sqlite3_column_text(stmt, 0));

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
        return result;
    }
public:
	DatabaseService(std::string db_file_path = "contacts.db") {
	   	if(sqlite3_open(db_file_path.c_str(), &db)) {
	      	std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
	   	}

        create_tables();
	}

    ~DatabaseService() {
	       sqlite3_close(db);
	}

	void create_tables() {
		char* err_msg = nullptr;
		char* sql_request = "CREATE TABLE IF NOT EXISTS users(id INTEGER PRIMARY KEY AUTOINCREMENT, login VARCHAR(32) NOT NULL UNIQUE, password VARCHAR(32) NOT NULL, cookie INTEGER DEFAULT NULL)";
		if(sqlite3_exec(db, sql_request, nullptr, nullptr, &err_msg) != SQLITE_OK){
			sqlite3_free(err_msg);
			std::cerr << "create_tables :: Create users table error: " << err_msg << std::endl;
      		throw std::string("Internal server error");
   		}

   		sql_request = "CREATE TABLE IF NOT EXISTS contacts(id INTEGER PRIMARY KEY AUTOINCREMENT, owner NOT NULL REFERENCES users(login), name VARCHAR(128) NOT NULL, email VARCHAR(128), phone_number VARCHAR(32))";
		if(sqlite3_exec(db, sql_request, nullptr, nullptr, &err_msg) != SQLITE_OK){
			sqlite3_free(err_msg);
			std::cerr << "create_tables :: Create contacts table error: " << err_msg << std::endl;
      		throw std::string("Internal server error");
   		}
	}

	void register_user(std::string login, std::string password) {
		sqlite3_stmt *stmt;

		if (sqlite3_prepare(db, "SELECT COUNT(*) FROM users WHERE login = ?", -1, &stmt, 0) != SQLITE_OK) {
    		std::cerr << "register_user :: Could not prepare statement" << std::endl;
    		throw std::string("Internal server error");
  		}

  		if (sqlite3_bind_text(stmt, 1, login.c_str(), -1, 0) != SQLITE_OK) {
  			sqlite3_finalize(stmt);
    		std::cerr << "register_user :: Could not bind login" << std::endl;
    		throw std::string("Internal server error");
  		}

  		int rc = sqlite3_step(stmt);
  		if (rc != SQLITE_ROW) {
  			sqlite3_reset(stmt);
    		sqlite3_finalize(stmt);
    		std::cerr << "register_user :: Could not step (execute) stmt" << std::endl;
    		throw std::string("Internal server error");
  		}

  		int count = sqlite3_column_int(stmt, 0);
  		if (count != 0) {
  			sqlite3_reset(stmt);
    		sqlite3_finalize(stmt);
    		std::cout << "Login already exists" << std::endl;
    		throw std::string("Login already exists");
  		}

  		sqlite3_reset(stmt);

		if (sqlite3_prepare(db, "INSERT INTO users(login, password) VALUES (?, ?)", -1, &stmt, 0) != SQLITE_OK) {
    		std::cerr << "register_user :: Could not prepare statement" << std::endl;
    		throw std::string("Internal server error");
  		}

  		if (sqlite3_bind_text(stmt, 1, login.c_str(), -1, 0) != SQLITE_OK) {
  			sqlite3_finalize(stmt);
    		std::cerr << "register_user :: Could not bind login" << std::endl;
    		throw std::string("Internal server error");
  		}

  		if (sqlite3_bind_text(stmt, 2, password.c_str(), -1, 0) != SQLITE_OK) {
  			sqlite3_reset(stmt);
    		sqlite3_finalize(stmt);
    		std::cerr << "register_user :: Could not bind password" << std::endl;
    		throw std::string("Internal server error");
  		}

  		if (sqlite3_step(stmt) != SQLITE_DONE) {
  			sqlite3_reset(stmt);
    		sqlite3_finalize(stmt);
    		std::cerr << "register_user :: Could not step (execute) stmt" << std::endl;
    		throw std::string("Internal server error");
  		}

  		sqlite3_reset(stmt);
		sqlite3_finalize(stmt);
	}

	int login_user(std::string login, std::string password) {
		sqlite3_stmt *stmt;

		if (sqlite3_prepare(db, "SELECT id FROM users WHERE login = ? and password = ?", -1, &stmt, 0) != SQLITE_OK) {
    		std::cerr << "login_user :: Could not prepare statement" << std::endl;
    		throw std::string("Internal server error");
  		}

  		if (sqlite3_bind_text(stmt, 1, login.c_str(), -1, 0) != SQLITE_OK) {
  			sqlite3_finalize(stmt);
    		std::cerr << "login_user :: Could not bind login" << std::endl;
    		throw std::string("Internal server error");
  		}

  		if (sqlite3_bind_text(stmt, 2, password.c_str(), -1, 0) != SQLITE_OK) {
    		sqlite3_reset(stmt);
    		sqlite3_finalize(stmt);
    		std::cerr << "login_user :: Could not bind password" << std::endl;
    		throw std::string("Internal server error");
  		}

  		int rc = sqlite3_step(stmt);
  		if (rc == SQLITE_DONE) {
  			sqlite3_reset(stmt);
    		sqlite3_finalize(stmt);
    		std::cout << "Credentials not found" << std::endl;
    		throw std::string("Credentials not found");
  		}
  		if (rc != SQLITE_ROW) {
    		sqlite3_reset(stmt);
    		sqlite3_finalize(stmt);
    		std::cerr << "login_user :: Could not step (execute) stmt" << std::endl;
    		throw std::string("Internal server error");
  		}

  		int id = sqlite3_column_int(stmt, 0);

  		sqlite3_reset(stmt);
    	sqlite3_finalize(stmt);

  		char* err_msg = nullptr;
  		std::string sql_request = "UPDATE users SET cookie = " + std::to_string(id) + " where id = " + std::to_string(id);

		if (sqlite3_exec(db, sql_request.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK) {
      		sqlite3_free(err_msg);
      		std::cerr << "login_user :: Update cookie error: " << err_msg << std::endl;
      		throw std::string("Internal server error");
   		}

   		return id;
	}

	void logout_user(int cookie) {
        sqlite3_stmt *stmt;

		if (sqlite3_prepare(db, "SELECT id FROM users WHERE cookie = ?", -1, &stmt, 0) != SQLITE_OK) {
    		std::cerr << "logout_user :: Could not prepare statement" << std::endl;
    		throw std::string("Internal server error");
  		}

  		if (sqlite3_bind_int(stmt, 1, cookie) != SQLITE_OK) {
    		sqlite3_finalize(stmt);
    		std::cerr << "logout_user :: Could not bind login" << std::endl;
    		throw std::string("Internal server error");
  		}

  		int rc = sqlite3_step(stmt);
  		if (rc == SQLITE_DONE) {
  			sqlite3_reset(stmt);
    		sqlite3_finalize(stmt);
    		std::cout << "Cookie not found" << std::endl;
    		throw std::string("Cookie not found");
  		}
  		if (rc != SQLITE_ROW) {
  			sqlite3_reset(stmt);
    		sqlite3_finalize(stmt);
    		std::cerr << "logout_user :: Could not step (execute) stmt" << std::endl;
    		throw std::string("Internal server error");
  		}

  		int id = sqlite3_column_int(stmt, 0);

  		sqlite3_reset(stmt);
    	sqlite3_finalize(stmt);

    	char* err_msg = nullptr;
  		std::string sql_request = "UPDATE users SET cookie = NULL where id = " + std::to_string(id);

		if (sqlite3_exec(db, sql_request.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK) {
			sqlite3_free(err_msg);
			std::cerr << "logout_user :: Update cookie error: " << err_msg << std::endl;
      		throw std::string("Internal server error");
   		}
	}

    std::vector<std::tuple<int, std::string, std::string, std::string>> get_contacts(int cookie) {
        std::string login = get_user_login(cookie);

        std::vector<std::tuple<int, std::string, std::string, std::string>> result;
        sqlite3_stmt *stmt;

        if (sqlite3_prepare(db, "SELECT id, name, email, phone_number FROM contacts WHERE owner=?", -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "get_contacts :: Could not prepare statement" << std::endl;
            throw std::string("Internal server error");
        }
        if (sqlite3_bind_text(stmt, 1, login.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "get_contacts :: Could not bind owner" << std::endl;
            throw std::string("Internal server error");
        }

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            return result;
        }
        if (rc != SQLITE_ROW) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "get_contacts :: Could not step (execute) stmt" << std::endl;
            throw std::string("Internal server error");
        }

        while(rc != SQLITE_DONE) {
            int id = sqlite3_column_int(stmt, 0);
            std::string name((char*)sqlite3_column_text(stmt, 1));
            std::string email((char*)sqlite3_column_text(stmt, 2));
            std::string phone_number((char*)sqlite3_column_text(stmt, 3));
            result.push_back(std::make_tuple(id, name, email, phone_number));
            rc = sqlite3_step(stmt);
        }

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);

        return result;
    }

    std::vector<std::tuple<int, std::string, std::string, std::string>> find_contacts_by_phone(int cookie, std::string phone_to_find) {
        std::string login = get_user_login(cookie);

        std::vector<std::tuple<int, std::string, std::string, std::string>> result;
        sqlite3_stmt *stmt;

        if (sqlite3_prepare(db, "SELECT id, name, email, phone_number FROM contacts WHERE owner=? and phone_number like ?", -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "get_contacts :: Could not prepare statement" << std::endl;
            throw std::string("Internal server error");
        }
        if (sqlite3_bind_text(stmt, 1, login.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "get_contacts :: Could not bind owner" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 2, phone_to_find.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "get_contacts :: Could not bind phone" << std::endl;
            throw std::string("Internal server error");
        }

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            return result;
        }
        if (rc != SQLITE_ROW) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "get_contacts :: Could not step (execute) stmt" << std::endl;
            throw std::string("Internal server error");
        }

        while(rc != SQLITE_DONE) {
            int id = sqlite3_column_int(stmt, 0);
            std::string name((char*)sqlite3_column_text(stmt, 1));
            std::string email((char*)sqlite3_column_text(stmt, 2));
            std::string phone_number((char*)sqlite3_column_text(stmt, 3));
            result.push_back(std::make_tuple(id, name, email, phone_number));
            rc = sqlite3_step(stmt);
        }

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);

        return result;
    }

    std::vector<std::tuple<int, std::string, std::string, std::string>> find_contacts_by_name(int cookie, std::string name_to_find) {
        std::string login = get_user_login(cookie);

        std::vector<std::tuple<int, std::string, std::string, std::string>> result;
        sqlite3_stmt *stmt;

        if (sqlite3_prepare(db, "SELECT id, name, email, phone_number FROM contacts WHERE owner=? and name like ?", -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "get_contacts :: Could not prepare statement" << std::endl;
            throw std::string("Internal server error");
        }
        if (sqlite3_bind_text(stmt, 1, login.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "get_contacts :: Could not bind owner" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 2, name_to_find.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "get_contacts :: Could not bind name" << std::endl;
            throw std::string("Internal server error");
        }

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            return result;
        }
        if (rc != SQLITE_ROW) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "get_contacts :: Could not step (execute) stmt" << std::endl;
            throw std::string("Internal server error");
        }

        while(rc != SQLITE_DONE) {
            int id = sqlite3_column_int(stmt, 0);
            std::string name((char*)sqlite3_column_text(stmt, 1));
            std::string email((char*)sqlite3_column_text(stmt, 2));
            std::string phone_number((char*)sqlite3_column_text(stmt, 3));
            result.push_back(std::make_tuple(id, name, email, phone_number));
            rc = sqlite3_step(stmt);
        }

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);

        return result;
    }

    std::vector<std::tuple<int, std::string, std::string, std::string>> find_contacts_by_email(int cookie, std::string email_to_find) {
        std::string login = get_user_login(cookie);

        std::vector<std::tuple<int, std::string, std::string, std::string>> result;
        sqlite3_stmt *stmt;

        if (sqlite3_prepare(db, "SELECT id, name, email, phone_number FROM contacts WHERE owner=? and email like ?", -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "get_contacts :: Could not prepare statement" << std::endl;
            throw std::string("Internal server error");
        }
        if (sqlite3_bind_text(stmt, 1, login.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "get_contacts :: Could not bind owner" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 2, email_to_find.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "get_contacts :: Could not bind email" << std::endl;
            throw std::string("Internal server error");
        }

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            return result;
        }
        if (rc != SQLITE_ROW) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "get_contacts :: Could not step (execute) stmt" << std::endl;
            throw std::string("Internal server error");
        }

        while(rc != SQLITE_DONE) {
            int id = sqlite3_column_int(stmt, 0);
            std::string name((char*)sqlite3_column_text(stmt, 1));
            std::string email((char*)sqlite3_column_text(stmt, 2));
            std::string phone_number((char*)sqlite3_column_text(stmt, 3));
            result.push_back(std::make_tuple(id, name, email, phone_number));
            rc = sqlite3_step(stmt);
        }

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);

        return result;
    }

    void add_contact(int cookie, std::string name, std::string email, std::string phone_number) {
        std::string owner = get_user_login(cookie);

        sqlite3_stmt *stmt;
        if (sqlite3_prepare(db, "INSERT INTO contacts(owner, name, email, phone_number) VALUES (?, ?, ?, ?)", -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "add_contact :: Could not prepare statement" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 1, owner.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "add_contact :: Could not bind owner" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 2, name.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "add_contact :: Could not bind name" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 3, email.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "add_contact :: Could not bind email" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 4, phone_number.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "add_contact :: Could not bind phone_number" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "add_contact :: Could not step (execute) stmt" << std::endl;
            throw std::string("Internal server error");
        }

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
    }

    void remove_contact(int cookie, int id) {
        std::string owner = get_user_login(cookie);

        sqlite3_stmt *stmt;
        if (sqlite3_prepare(db, "DELETE FROM contacts WHERE id=? AND owner=?", -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "remove_contact :: Could not prepare statement" << std::endl;
            throw std::string("Internal server error");
        }
        if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "remove_contact :: Could not bind id" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 2, owner.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "remove_contact :: Could not bind owner" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "remove_contact :: Could not step (execute) stmt" << std::endl;
            throw std::string("Internal server error");
        }

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
    }

    void update_contact(int cookie, int id, std::string name, std::string email, std::string phone_number) {
        std::string owner = get_user_login(cookie);

        sqlite3_stmt *stmt;
        if (sqlite3_prepare(db, "UPDATE contacts SET name=?, email=?, phone_number=? WHERE id=? AND owner=?", -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "update_contact :: Could not prepare statement" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 1, name.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "update_contact :: Could not bind name" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 2, email.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "update_contact :: Could not bind email" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 3, phone_number.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "update_contact :: Could not bind phone_number" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_int(stmt, 4, id) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "update_contact :: Could not bind id" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 5, owner.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "update_contact :: Could not bind owner" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "update_contact :: Could not step (execute) stmt" << std::endl;
            throw std::string("Internal server error");
        }

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
    }
};

// int main() {
// 	DatabaseService service("contacts.db");
//
// 	try {
// 		service.create_tables();
//
//         int cookie;
// 		for (int i = 0; i < 10; ++i) {
// 			service.register_user(std::to_string(i), std::to_string(i));
// 			cookie = service.login_user(std::to_string(i), std::to_string(i));
//             if (i + 1 == 10) break;
// 			service.logout_user(cookie);
// 		}
//
//         service.add_contact(cookie, "Boris", "borisVodka@soviet.sssr", "+7909889898");
//         service.add_contact(cookie, "John", "johnWiskey@states.com", "+3202839491");
//
//         service.update_contact(cookie, 1, "Ivan", "ivanVodka@soviet.sssr", "+7909881111");
//         service.remove_contact(cookie, 2);
//
// 		std::vector<std::tuple<int, std::string, std::string, std::string>> contacts = service.get_contacts(cookie);
// 		for (int i = 0; i < contacts.size(); ++i) {
// 			std::cout << "Id=" << std::get<0>(contacts[i]) << "; ";
//             std::cout << "Name=" << std::get<1>(contacts[i]) << "; ";
//             std::cout << "Email=" << std::get<2>(contacts[i]) << "; ";
//             std::cout << "Phone_number=" << std::get<3>(contacts[i]) << std::endl;
// 		}
// 	} catch(std::string descr) {
// 		std::cout << descr << std::endl;
// 	}
// }

// g++ -std=c++11 database.cpp -o database -lsqlite3 -w
