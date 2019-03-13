#include <sqlite3.h>
#include <string>
#include <iostream>
#include <vector>

class DatabaseService {
private:
  sqlite3* db;
public:
	DatabaseService(std::string db_file_path = "chat.db") {
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

   		sql_request = "CREATE TABLE IF NOT EXISTS messages(id INTEGER PRIMARY KEY AUTOINCREMENT, sender NOT NULL REFERENCES users(login), receiver NOT NULL REFERENCES users(login), message VARCHAR(128) NOT NULL, timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL)";
		if(sqlite3_exec(db, sql_request, nullptr, nullptr, &err_msg) != SQLITE_OK){
			sqlite3_free(err_msg);
			std::cerr << "create_tables :: Create messages table error: " << err_msg << std::endl;
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

	std::vector<std::pair<std::string, bool>> users_list() {
		std::vector<std::pair<std::string, bool>> result;
		sqlite3_stmt *stmt;

		if (sqlite3_prepare(db, "SELECT login, cookie FROM users", -1, &stmt, 0) != SQLITE_OK) {
    		std::cerr << "online_users_list :: Could not prepare statement" << std::endl;
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
    		std::cerr << "online_users_list :: Could not step (execute) stmt" << std::endl;
    		throw std::string("Internal server error");
  		}

  		while(rc != SQLITE_DONE) {
  			std::string result_elem((char*)sqlite3_column_text(stmt, 0));
            int status = sqlite3_column_int(stmt, 1);
  			result.push_back(std::make_pair(result_elem, status != NULL));
  			rc = sqlite3_step(stmt);
  		}

  		sqlite3_reset(stmt);
		sqlite3_finalize(stmt);

  		return result;
	}

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

    std::vector<std::pair<std::string, std::string>> get_messages(std::string login) {
        std::vector<std::pair<std::string, std::string>> result;
        sqlite3_stmt *stmt;

        if (sqlite3_prepare(db, "SELECT sender, message FROM messages WHERE receiver=?", -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "get_user_message :: Could not prepare statement" << std::endl;
            throw std::string("Internal server error");
        }
        if (sqlite3_bind_text(stmt, 1, login.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "get_user_message :: Could not bind login" << std::endl;
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
            std::cerr << "get_user_message :: Could not step (execute) stmt" << std::endl;
            throw std::string("Internal server error");
        }

        while(rc != SQLITE_DONE) {
            receive_message buf;
            std::string from((char*)sqlite3_column_text(stmt, 0));
            std::string message((char*)sqlite3_column_text(stmt, 1));
            result.push_back(std::make_pair(from ,message));
            rc = sqlite3_step(stmt);
        }

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);

        return result;
    }

    void add_message(std::string receiver_login, std::string sender_login, std::string message) {
        sqlite3_stmt *stmt;
        if (sqlite3_prepare(db, "INSERT INTO messages(sender, receiver, message) VALUES (?, ?, ?)", -1, &stmt, 0) != SQLITE_OK) {
            std::cerr << "add_message :: Could not prepare statement" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 1, sender_login.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            std::cerr << "add_message :: Could not bind sender" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 2, receiver_login.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "add_message :: Could not bind receiver" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_bind_text(stmt, 3, message.c_str(), -1, 0) != SQLITE_OK) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "add_message :: Could not bind message" << std::endl;
            throw std::string("Internal server error");
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_reset(stmt);
            sqlite3_finalize(stmt);
            std::cerr << "add_message :: Could not step (execute) stmt" << std::endl;
            throw std::string("Internal server error");
        }

        sqlite3_reset(stmt);
        sqlite3_finalize(stmt);
    }
};

// int main() {
// 	DatabaseService service(DB_PATH);

// 	try {
// 		service.create_tables();

// 		for (int i = 0; i < 10; ++i) {
// 			service.register_user(std::to_string(i), std::to_string(i));
// 			int cookie = service.login_user(std::to_string(i), std::to_string(i));
// 			service.logout_user(cookie);
// 		}

// 		std::vector<std::pair<std::string, bool>> users = service.users_list();
// 		for (int i = 0; i < users.size(); ++i) {
// 			std::cout << users[i].first << std::endl;
//       std::cout << users[i].second << std::endl;
// 		}
// 	} catch(std::string descr) {
// 		std::cout << descr << std::endl;
// 	}
// }

// g++ -std=c++11 database.cpp -o database -lsqlite3 -w