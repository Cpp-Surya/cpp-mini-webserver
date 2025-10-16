#include <atomic>
#include <csignal>
#include <iostream>
#include <memory>

#include "MiniWebServer/socket_wrapper_impl.h"
#include "MiniWebServer/webserver.h"

std::shared_ptr<WebServer> global_server = nullptr;

void signalHandler(int signum)
{
    std::cout << "\nStopping server...\n";
    if (global_server != nullptr)
    {
        global_server->shutdownSocket();
        global_server.reset();
    }
}

// Simple single-threaded HTTP server.
// Usage: ./mini_web_server [port]
int main(int argc, char** argv)
{
    try
    {
        int port = 8080;

        if (argc > 1)
        {
            port = std::stoi(argv[1]);
        }

        if (port < 1024 || port > 49151)
        {
            throw std::runtime_error("Trying to use restricted ports");
        }

        std::shared_ptr<ISocketWrapper> socket_ptr = std::make_shared<SocketWrapperImpl>();
        std::shared_ptr<WebServer> server = std::make_shared<WebServer>(*socket_ptr);

        std::signal(SIGINT, signalHandler);
        global_server = server;

        server->setSocketOptions();

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        server->bindSocket(addr);
        server->listenConnections();
        server->acceptConnections();
    }
    catch (std::exception& e)
    {
        std::cerr << "Error---> " << e.what() << "\n";
        return -1;
    }
    return 0;
}