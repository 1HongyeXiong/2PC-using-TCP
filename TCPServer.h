#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <string>

class TCPServer {
protected:
    int server;
    int client;
    virtual void start_client(const std::string &their_host, unsigned short their_port) = 0;
    virtual bool process(const std::string &request, int client_socket) = 0;
    void handle_client(int client_socket);

public:
    TCPServer(unsigned short port);
    virtual ~TCPServer();
    void serve();
    void respond(int client_socket, const std::string &response);
};

#endif // TCPSERVER_H
