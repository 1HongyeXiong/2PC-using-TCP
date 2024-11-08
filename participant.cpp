#include <iostream>
#include "BankServer.h"
#include <string>

// Main function to run the BankServer
int main(int argc, char* argv[]) {
    // Ensure the correct number of command-line arguments are provided
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <port> <accountsFile> <logFile>\n";
        return EXIT_FAILURE;
    }

    // Parse the command-line arguments
    int port = std::stoi(argv[1]); // Convert port from string to int
    std::string accountsFile = argv[2]; // Account file path
    std::string logFile = argv[3]; // Log file path
    Logger logger(logFile); // Initialize the logger
    // Log that the transaction service has started
    logger.log("Transaction service on port " + std::to_string(port) + " (Ctrl-c to stop)");

    try {
        // Create a BankServer instance and start serving
        BankServer server(port, accountsFile, logFile);
        server.serve(); // Assuming TCPServer class includes a method to start listening
    } catch (const std::exception& e) {
        // Catch and log any exceptions that occur during server startup
        std::cerr << "Failed to start the server: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
