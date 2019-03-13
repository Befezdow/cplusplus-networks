#include <iostream>
#include <functional>
#include <map>
#include <queue>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/lexical_cast.hpp>
#include "posix_thread_wrapper.h"
#include "handlers.h"

using namespace std;

class http_server{
    std::queue<int>clients;
    std::vector<m_thread::thread*>threads;
    m_thread::mutex mtx;
    m_thread::condition_variable cond_var;
    int sock;

public:
    http_server(int port,int poll_size) : mtx(m_thread::mutex::Normal){
        struct sockaddr_in ss_addr;

        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sock == -1)perror("Error creating socket");

        setsockopt(sock, 1, SO_REUSEADDR, 0, 0);

        ss_addr.sin_family = AF_INET;
        ss_addr.sin_addr.s_addr = INADDR_ANY;
        ss_addr.sin_port = htons(port);

        if(bind(sock, (struct sockaddr *) &ss_addr, sizeof(ss_addr)) != 0){perror("Error binding socket\n");};
        if (listen(sock, 10) != 0)perror("Error listen socket");

        for(int i(0);i != poll_size;++i)threads.push_back(new m_thread::thread(m_thread::thread::Detached,&http_server::thread_handle,&clients,&mtx,&cond_var));
    }
    void start(){
        for(;;) {

            struct sockaddr_in cs_addr;
            socklen_t cs_len = sizeof(cs_addr);

            int cs = accept(sock, (struct sockaddr *) &cs_addr, &cs_len);
            if (cs != -1) {
                mtx.lock();
                clients.push(cs);
                cond_var.notify_one();
                mtx.unlock();
            }

        }
    }
    ~http_server() {
        close(sock);
        for(auto it : threads) delete it;
    }

private:

    static void thread_handle(std::queue<int> *clients,m_thread::mutex *mtx, m_thread::condition_variable *cond_var) {
        for(;;) {

            int sock;
            mtx->lock();
            while(!clients->size()){
                cond_var->wait(*mtx);
            }
            sock = clients->back();
            clients->pop();
            mtx->unlock();

            Handlers handlers;
            handlers.handle_request(sock);
        }
    }
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Incorrect count of params. Need to specify only port";
        return 1;
    }
    std::cout << "Server is on " << argv[1] << std::endl;
    int port_num = boost::lexical_cast<int>(argv[1]);
    http_server server(port_num, 8);
    server.start();
    return 0;
}

// compiled with: g++ (GCC) 7.3.1 20180303 (Red Hat 7.3.1-5)
// scl enable devtoolset-7 bash
// g++ main.cpp -o server -lsqlite3 -pthread -w
