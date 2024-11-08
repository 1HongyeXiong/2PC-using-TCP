#ifndef TCPP2_COORDINATORCLIENT_H
#define TCPP2_COORDINATORCLIENT_H

#include "TCPClient.h"
#include "Logger.h"
#include <string>

// The CoordinatorClient class inherits from TCPClient to handle specific networking and logging tasks for transaction coordination.
class CoordinatorClient : public TCPClient {
private:
    std::string host; // Host address of the coordinator
    int port; // Port number on which the coordinator service is available

public:
    // Constructor to initialize a CoordinatorClient with specified host, port, and log file
    CoordinatorClient(const std::string& host, int port, const std::string& logFilename)
            : TCPClient(host, port, logFilename), host(host), port(port) {}

    // Method to send a vote request and receive a response
    bool requestVote(const std::string& transactionID, const std::string& account, double amount) {
        // Constructing the vote request message
        std::string message = "VOTE-REQUEST " + transactionID + " " + account + " " + std::to_string(amount);
        logger.log("Sending message '" + message + "' to " + get_host() + ":" + std::to_string(get_port()));
        send_request(message); // Sending the request to the coordinator
        std::string response = get_response();  // Receiving the participant's response

        // Returning true if the response is a commit vote
        return response == "VOTE-COMMIT " + transactionID;  // Check if response is approval
    }

    // Getter method for host
    std::string get_host() const {
        return host;
    }

    // Getter method for port
    int get_port() const {
        return port;
    }

    // Method to send a global commit or abort command based on the vote result
    void finalizeTransaction(const std::string& transactionID, bool commit, const std::string& account, double amount) {
        // Deciding the command based on the commit flag
        std::string command = commit ? "GLOBAL-COMMIT" : "GLOBAL-ABORT";
        // Constructing the message to be sent
        std::string message = command + " " + transactionID + " " + account + " " + std::to_string(amount);
        logger.log("Sending message '" + message + "' to " + get_host() + ":" + std::to_string(get_port()));
        send_request(message); // Sending the transaction finalization command
        std::string response = get_response();  // Expecting an ACK or another confirmation

    }
};

#endif // TCPP2_COORDINATORCLIENT_H
