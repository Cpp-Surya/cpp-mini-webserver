#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "MiniWebServer/socket_wrapper.h"
#include "MiniWebServer/webserver.h"

class MockSocketWrapper : public ISocketWrapper
{
   public:
    MOCK_METHOD(int, socket_create, (int, int, int), (override));
    MOCK_METHOD(int, socket_options, (int, int, int, const void*, socklen_t), (override));
    MOCK_METHOD(int, socket_bind, (int, const struct sockaddr*, socklen_t), (override));
    MOCK_METHOD(int, socket_listen, (int, int), (override));
    MOCK_METHOD(int, socket_accept, (int, struct sockaddr*, socklen_t*), (override));
    MOCK_METHOD(ssize_t, socket_send, (int, const void*, size_t, int), (override));
    MOCK_METHOD(ssize_t, socket_recv, (int, void*, size_t, int), (override));
    MOCK_METHOD(int, socket_shutdown, (int, int), (override));
    MOCK_METHOD(int, socket_close, (int), (override));
};

class WebServerTest : public ::testing::Test
{
   protected:
    void TearDown() override
    {
        if (server != nullptr)
        {
            server->shutdownSocket();
            server.reset();
        }
    }

    MockSocketWrapper mock;
    std::unique_ptr<WebServer> server;
};

TEST_F(WebServerTest, Constructor_CreatesSocketSuccessfully)
{
    EXPECT_CALL(mock, socket_create(AF_INET, SOCK_STREAM, 0)).WillOnce(testing::Return(3));
    EXPECT_CALL(mock, socket_shutdown(3, SHUT_RDWR)).WillOnce(testing::Return(0));
    EXPECT_CALL(mock, socket_close(3)).WillOnce(testing::Return(0));

    server = std::make_unique<WebServer>(mock);
}

TEST_F(WebServerTest, Constructor_ThrowsOnSocketCreateFailure)
{
    EXPECT_CALL(mock, socket_create).WillOnce(testing::Return(-1));
    EXPECT_THROW(server = std::make_unique<WebServer>(mock), std::runtime_error);
}

TEST_F(WebServerTest, SetSocketOptions_Success)
{
    EXPECT_CALL(mock, socket_create).WillOnce(testing::Return(3));
    EXPECT_CALL(mock, socket_options).WillOnce(testing::Return(0));
    EXPECT_CALL(mock, socket_shutdown).WillOnce(testing::Return(0));
    EXPECT_CALL(mock, socket_close).WillOnce(testing::Return(0));

    server = std::make_unique<WebServer>(mock);
    EXPECT_NO_THROW(server->setSocketOptions());
}

TEST_F(WebServerTest, SetSocketOptions_FailureThrows)
{
    EXPECT_CALL(mock, socket_create).WillOnce(testing::Return(5));
    EXPECT_CALL(mock, socket_options).WillOnce(testing::Return(-1));
    EXPECT_CALL(mock, socket_shutdown).WillOnce(testing::Return(0));
    EXPECT_CALL(mock, socket_close).WillOnce(testing::Return(0));

    server = std::make_unique<WebServer>(mock);
    EXPECT_THROW(server->setSocketOptions(), std::runtime_error);
}

TEST_F(WebServerTest, BindSocket_Success)
{
    EXPECT_CALL(mock, socket_create).WillOnce(testing::Return(3));
    EXPECT_CALL(mock, socket_bind).WillOnce(testing::Return(0));
    EXPECT_CALL(mock, socket_shutdown).WillOnce(testing::Return(0));
    EXPECT_CALL(mock, socket_close).WillOnce(testing::Return(0));

    server = std::make_unique<WebServer>(mock);
    sockaddr_in addr{};
    EXPECT_NO_THROW(server->bindSocket(addr));
}

TEST_F(WebServerTest, BindSocket_FailureThrows)
{
    EXPECT_CALL(mock, socket_create).WillOnce(testing::Return(3));
    EXPECT_CALL(mock, socket_bind).WillOnce(testing::Return(-1));
    EXPECT_CALL(mock, socket_shutdown).WillOnce(testing::Return(0));
    EXPECT_CALL(mock, socket_close).WillOnce(testing::Return(0));

    server = std::make_unique<WebServer>(mock);
    sockaddr_in addr{};
    EXPECT_THROW(server->bindSocket(addr), std::runtime_error);
}

TEST_F(WebServerTest, ListenConnections_Success)
{
    EXPECT_CALL(mock, socket_create).WillOnce(testing::Return(3));
    EXPECT_CALL(mock, socket_listen).WillOnce(testing::Return(0));
    EXPECT_CALL(mock, socket_shutdown).WillOnce(testing::Return(0));
    EXPECT_CALL(mock, socket_close).WillOnce(testing::Return(0));

    server = std::make_unique<WebServer>(mock);
    EXPECT_NO_THROW(server->listenConnections());
}

TEST_F(WebServerTest, ListenConnections_FailureThrows)
{
    EXPECT_CALL(mock, socket_create).WillOnce(testing::Return(3));
    EXPECT_CALL(mock, socket_listen).WillOnce(testing::Return(-1));
    EXPECT_CALL(mock, socket_shutdown).WillOnce(testing::Return(0));
    EXPECT_CALL(mock, socket_close).WillOnce(testing::Return(0));

    server = std::make_unique<WebServer>(mock);
    EXPECT_THROW(server->listenConnections(), std::runtime_error);
}

TEST_F(WebServerTest, AcceptConnections_HandlesClientAndStopsAfterFailure)
{
    EXPECT_CALL(mock, socket_create).WillOnce(testing::Return(3));
    EXPECT_CALL(mock, socket_accept)
        .WillOnce(testing::Return(-1))  // failure
        .WillOnce(testing::Return(5));  // success

    EXPECT_CALL(mock, socket_recv)
        .WillOnce(testing::Return(10))  // received some data
        .WillOnce(testing::Return(0));  // end recv loop

    EXPECT_CALL(mock, socket_send).Times(1);
    EXPECT_CALL(mock, socket_shutdown).Times(1);

    EXPECT_CALL(mock, socket_close).Times(2);

    server = std::make_unique<WebServer>(mock);
    server->acceptConnections(1);  // limit loop to 1 client
}

TEST_F(WebServerTest, SocketShutdown_FailureThrows)
{
    EXPECT_CALL(mock, socket_create).WillOnce(testing::Return(3));
    EXPECT_CALL(mock, socket_close).WillOnce(testing::Return(0));
    EXPECT_CALL(mock, socket_shutdown).WillOnce(testing::Return(-1));

    server = std::make_unique<WebServer>(mock);
    EXPECT_THROW(server->shutdownSocket(), std::runtime_error);
    server.reset();
}

TEST_F(WebServerTest, Destructor_ClosesSocket)
{
    EXPECT_CALL(mock, socket_create).WillOnce(testing::Return(3));
    EXPECT_CALL(mock, socket_close(3)).Times(1);

    {
        WebServer local_server(mock);  // destructor runs at scope end
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}