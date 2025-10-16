#ifndef SOCKET_WRAPPER_H_
#define SOCKET_WRAPPER_H_

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

class ISocketWrapper
{
   public:
    virtual ~ISocketWrapper() = default;

    virtual int socket_create(int domain, int type, int protocol) = 0;
    virtual int socket_options(int sockfd, int level, int optname, const void* optval,
                               socklen_t optlen) = 0;
    virtual int socket_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) = 0;
    virtual int socket_listen(int sockfd, int backlog) = 0;
    virtual int socket_accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) = 0;
    virtual ssize_t socket_send(int sockfd, const void* buf, size_t len, int flags) = 0;
    virtual ssize_t socket_recv(int sockfd, void* buf, size_t n, int flags) = 0;
    virtual int socket_shutdown(int sockfd, int how) = 0;
    virtual int socket_close(int fd) = 0;
};

#endif  // SOCKET_WRAPPER_H_
