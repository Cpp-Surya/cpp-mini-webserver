#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <atomic>
#include <iostream>
#include <stdexcept>
#include <string>

#include "MiniWebServer/socket_wrapper.h"
#include "MiniWebServer/thread_pool.h"

class WebServer
{
   public:
    explicit WebServer(ISocketWrapper& socket_api);
    virtual ~WebServer();
    void setSocketOptions();
    void bindSocket(const sockaddr_in& addr);
    void listenConnections();
    void acceptConnections(int max_clients = -1);
    void stopServer();
    void handleClient(int client_fd);

   private:
    ISocketWrapper& socket_api_;
    ThreadPool thread_pool_;
    int server_fd_{-1};
    std::atomic<bool> running_{false};
};

#endif  // WEB_SERVER_H_
