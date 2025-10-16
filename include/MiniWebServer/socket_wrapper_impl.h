#ifndef SOCKET_WRAPPER_IMPL_H_
#define SOCKET_WRAPPER_IMPL_H_

#include "socket_wrapper.h"

class SocketWrapperImpl : public ISocketWrapper
{
   public:
    int socket_create(int domain, int type, int protocol) override
    {
        return ::socket(domain, type, protocol);
    }

    int socket_options(int sockfd, int level, int optname, const void* optval,
                       socklen_t optlen) override
    {
        return ::setsockopt(sockfd, level, optname, optval, optlen);
    }

    int socket_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) override
    {
        return ::bind(sockfd, addr, addrlen);
    }

    int socket_listen(int sockfd, int backlog) override { return ::listen(sockfd, backlog); }

    int socket_accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) override
    {
        return ::accept(sockfd, addr, addrlen);
    }

    ssize_t socket_send(int sockfd, const void* buf, size_t len, int flags) override
    {
        return ::send(sockfd, buf, len, flags);
    }

    ssize_t socket_recv(int sockfd, void* buf, size_t len, int flags) override
    {
        return ::recv(sockfd, buf, len, flags);
    }

    int socket_shutdown(int sockfd, int how) override { return ::shutdown(sockfd, how); }

    int socket_close(int fd) override { return ::close(fd); }
};

#endif  // SOCKET_WRAPPER_IMPL_H_