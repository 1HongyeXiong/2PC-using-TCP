#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <thread>
#include "TCPServer.h"
using namespace std;

TCPServer::TCPServer(unsigned short port) {
    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
        throw runtime_error(strerror(errno));
    if (port != 0) {
        sockaddr_in me = {};
        me.sin_family = AF_INET;
        me.sin_port = htons(port);
        me.sin_addr.s_addr = inet_addr("0.0.0.0");
        if (::bind(server, (sockaddr *)&me, sizeof(me)) < 0)
            throw runtime_error(strerror(errno));
    }
    listen(server, 1);
    client = -1;
}

TCPServer::~TCPServer() {
    close(server);
}

void TCPServer::serve() {
    while (true) {
        sockaddr_in them = {};
        socklen_t them_len = sizeof(them);
        int client_socket = accept(server, (sockaddr *) &them, &them_len);
        if (client_socket < 0) {
            throw runtime_error(strerror(errno));
        }

        char *their_host = inet_ntoa(them.sin_addr);
        unsigned short their_port = ntohs(them.sin_port);
        start_client(their_host, their_port);

        std::thread(&TCPServer::handle_client, this, client_socket).detach();
    }
}

void TCPServer::handle_client(int client_socket) {
    while (true) {
        char buffer[1024];
        ssize_t received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (received < 0) {
            close(client_socket);
            return;
        }
        buffer[received] = '\0';  // null-terminate c-style string
        string request(buffer);
        if (!process(request, client_socket)) {
            close(client_socket);
            return;
        }
    }
}

void TCPServer::respond(int client_socket, const string &response) {
    ssize_t sent = send(client_socket, response.c_str(), response.length(), 0);
    if (sent < 0) {
        throw runtime_error(strerror(errno));
    }
}
