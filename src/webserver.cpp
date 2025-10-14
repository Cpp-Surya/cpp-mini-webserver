#include "MiniWebServer/webserver.h"

WebServer::WebServer(ISocketWrapper& socket_api) : socket_api_(socket_api)
{
    server_fd_ = socket_api_.socket_create(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == -1)
    {
        throw std::runtime_error("Socket creation failed");
    }
}

WebServer::~WebServer()
{
    if (server_fd_ != -1)
    {
        socket_api_.socket_close(server_fd_);
        server_fd_ = -1;
    }
}

void WebServer::setSocketOptions()
{
    int opt = 1;
    if (socket_api_.socket_options(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        throw std::runtime_error("Set socket options failed");
    }
}

void WebServer::bindSocket(const sockaddr_in& addr)
{
    if (socket_api_.socket_bind(server_fd_, reinterpret_cast<const sockaddr*>(&addr),
                                sizeof(addr)) < 0)
    {
        throw std::runtime_error("Bind failed");
    }
}

void WebServer::listenConnections()
{
    if (socket_api_.socket_listen(server_fd_, SOMAXCONN) < 0)
    {
        throw std::runtime_error("Listen failed");
    }
}

void WebServer::acceptConnections(int max_clients)
{
    running_ = true;
    int count = 0;
    while (running_ && (max_clients == -1 || count < max_clients))
    {
        sockaddr_in client{};
        socklen_t len = sizeof(client);
        int client_fd =
            socket_api_.socket_accept(server_fd_, reinterpret_cast<sockaddr*>(&client), &len);

        if (!running_)
        {
            break;
        }

        if (client_fd < 0)
        {
            if (running_)
            {
                continue;
            }
            else
            {
                break;
            }
        }

        count++;  // increment for testability
        std::string request = "";
        char buf[4096];
        ssize_t n;
        while ((n = socket_api_.socket_recv(client_fd, buf, sizeof(buf), 0)) > 0)
        {
            request.append(buf, buf + n);
            if (request.find("\r\n\r\n") != std::string::npos)
                break;
        }

        std::cout << "---- Request start ----\n" << request << "---- Request end ----\n";
        const std::string body =
            "<html><body><h1>Hello World</h1><p>Mini C++ server</p></body></html>";
        std::string response = "";
        response += "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html; charset=utf-8\r\n";
        response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += body;
        socket_api_.socket_send(client_fd, response.c_str(), response.size(), 0);
        socket_api_.socket_close(client_fd);
    }
}

void WebServer::shutdownSocket()
{
    if (server_fd_ != -1)
    {
        if (socket_api_.socket_shutdown(server_fd_, SHUT_RDWR) < 0)
        {
            throw std::runtime_error("Socket shutdown failed");
        }
    }
    running_ = false;
}
