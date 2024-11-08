// Guard to prevent multiple inclusions of this header
#ifndef TCPP2_BANKSERVER_H
#define TCPP2_BANKSERVER_H

// Including required header files
#include "TCPServer.h"
#include "Logger.h"
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <mutex>
#include <map>

// Declaration of the BankServer class, inheriting from TCPServer
class BankServer : public TCPServer {
    Logger logger; // Instance of Logger to handle logging
    std::unordered_map<std::string, double> accounts; // Map to store account balances
    std::string accountsFile; // File path for storing account details
    std::mutex mtx;  // Mutex to protect the accounts data
    std::map<std::string, std::map<std::string, double>> transactions; // Map for transaction ID to account and amount mapping

public:
    // Constructor to initialize the BankServer with a specific port, accounts file, and log filename
    BankServer(int port, const std::string& accountsFile, const std::string& logFilename)
            : TCPServer(port), logger(logFilename), accountsFile(accountsFile) {
        loadAccounts(accountsFile); // Loading accounts from file
    }

protected:
    // Function to load accounts from a file
    void loadAccounts(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open accounts file: " + filename);
        }

        std::string line;
        while (getline(file, line)) {
            std::istringstream iss(line);
            double balance;
            std::string account;
            if (iss >> balance >> account) {
                accounts[account] = balance;
            }
        }
    }
    // Function to save account balances to a file
    void saveAccounts() {
        std::ofstream file(accountsFile, std::ios::out | std::ios::trunc);
        if (!file) {
            logger.log("Failed to open accounts file for saving: " + accountsFile);
            return;
        }

        for (const auto& pair : accounts) {
            file << pair.second << " " << pair.first << "\n";
            if (file.fail()) {
                logger.log("Failed to write account: " + pair.first);
                return;
            }
        }

        file.close();
        if (file.fail()) {
            logger.log("Failed to close accounts file: " + accountsFile);
        }
    }

    // Override function to handle new client connections
    virtual void start_client(const std::string &their_host, unsigned short their_port) override {
        logger.log("Accepting coordinator connection. State: INIT");
    }

    // Override function to process incoming requests
    virtual bool process(const std::string &request, int client_socket) override {
        std::istringstream iss(request);
        std::string command, transactionID, account;
        double amount;

        iss >> command >> transactionID >> account >> amount;

        if (command == "VOTE-REQUEST") {
            std::lock_guard<std::mutex> lock(mtx);
            if (amount > 0) {
                logger.log("Holding " + std::to_string(-amount) + " from the account " + account);
            }

            if (accounts.find(account) == accounts.end() || (amount < 0 && accounts[account] + amount < 0)) {
                respond(client_socket, "VOTE-ABORT " + transactionID);
                logger.log("Got VOTE-REQUEST, replying VOTE-ABORT. State: ABORT");
                return true;
            } else {
                transactions[transactionID][account] = amount;
                respond(client_socket, "VOTE-COMMIT " + transactionID);
                logger.log("Got VOTE-REQUEST, replying VOTE-COMMIT. State: READY");
                return true;
            }
        } else if (command == "GLOBAL-COMMIT") {
            std::lock_guard<std::mutex> lock(mtx);

            if (transactions.find(transactionID) != transactions.end()) {
                for (const auto& trans : transactions[transactionID]) {
                    accounts[trans.first] += trans.second;
                }

                transactions.erase(transactionID);

                saveAccounts();
                logger.log("Got GLOBAL-COMMIT, replying ACK. State: COMMIT");

                respond(client_socket, "ACK " + transactionID);
                if (amount > 0) {
                    logger.log("Releasing hold from the account " + account);
                } else {
                    logger.log("Releasing hold from the account");
                }

                return true;
            } else {
                logger.log("Transaction ID not found: " + transactionID);
                respond(client_socket, "ACK " + transactionID);
                return true;
            }
        } else if (command == "GLOBAL-ABORT") {
            std::lock_guard<std::mutex> lock(mtx);
            transactions.erase(transactionID);
            respond(client_socket, "ACK " + transactionID);
            logger.log("Got GLOBAL-ABORT, replying ACK. State: ABORT");
            logger.log("Recovering the hold from the account " + account);

            return true;
        }
        return true;
    }
};

#endif // TCPP2_BANKSERVER_H
