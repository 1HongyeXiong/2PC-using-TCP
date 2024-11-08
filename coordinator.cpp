#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include "CoordinatorClient.h"
#include "Logger.h"

// Global transaction queue and mutex for synchronization
std::queue<std::tuple<std::string, double, std::string, int, std::string, std::string, int, std::string>> transactionQueue;
std::mutex mtx;
std::condition_variable cv;
bool stop = false; // Control variable for stopping the transaction handler

// Maps to keep track of client connections and connected ports
std::unordered_map<int, CoordinatorClient*> clientConnections; // Connection pool
std::unordered_set<int> connectedPorts; // Set of connected ports

// Function to handle transactions; it processes each transaction from the global queue
void handleTransaction(const std::string& logFile) {
    Logger logger(logFile);

    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        // Wait until there's something in the queue or stop is signaled
        cv.wait(lock, [] { return !transactionQueue.empty() || stop; });

        // If stop is true and the queue is empty, exit the loop
        if (stop && transactionQueue.empty()) {
            return;
        }

        // Process the transaction at the front of the queue
        auto transaction = transactionQueue.front();
        transactionQueue.pop();
        lock.unlock();

        // Extract transaction details
        std::string transactionID = std::get<0>(transaction);
        double amount = std::get<1>(transaction);
        std::string sourceHost = std::get<2>(transaction);
        int sourcePort = std::get<3>(transaction);
        std::string sourceAccount = std::get<4>(transaction);
        std::string destHost = std::get<5>(transaction);
        int destPort = std::get<6>(transaction);
        std::string destAccount = std::get<7>(transaction);

        CoordinatorClient* sourceClient;
        CoordinatorClient* destClient;

        // Create or get client connections for the source and destination
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (connectedPorts.find(sourcePort) == connectedPorts.end()) {
                sourceClient = new CoordinatorClient(sourceHost, sourcePort, logFile);
                clientConnections[sourcePort] = sourceClient;
                connectedPorts.insert(sourcePort);
            } else {
                sourceClient = clientConnections[sourcePort];
            }

            if (connectedPorts.find(destPort) == connectedPorts.end()) {
                destClient = new CoordinatorClient(destHost, destPort, logFile);
                clientConnections[destPort] = destClient;
                connectedPorts.insert(destPort);
            } else {
                destClient = clientConnections[destPort];
            }
        }

        // Log the start of the transaction
        logger.log("Starting transaction: " + transactionID);

        // Attempt to get approval for the transaction from both source and destination
        bool sourceApproved = sourceClient->requestVote(transactionID, sourceAccount, -amount);
        bool destApproved = destClient->requestVote(transactionID, destAccount, amount);

        // Finalize the transaction based on approvals
        if (sourceApproved && destApproved) {
            sourceClient->finalizeTransaction(transactionID, true, sourceAccount, -amount);
            destClient->finalizeTransaction(transactionID, true, destAccount, amount);
            logger.log("Transaction " + transactionID + " committed.\n");
        } else {
            sourceClient->finalizeTransaction(transactionID, false, sourceAccount, -amount);
            destClient->finalizeTransaction(transactionID, false, destAccount, amount);
            logger.log("Transaction " + transactionID + " failed or aborted.\n");
        }
    }
}

int main(int argc, char* argv[]) {
    // Check for proper command-line argument count
    if (argc < 9 || (argc - 9) % 7 != 0) {
        std::cerr << "Usage: " << argv[0]
                  << " <logFile> <initialAmount> <initialSourceHost> <initialSourcePort> <initialSourceAccount> <initialDestHost> <initialDestPort> <initialDestAccount> [<amount> <participantHost> <participantPort> <participantSourceAccount> <participantDestHost> <participantDestPort> <participantDestAccount> ...]\n";
        return EXIT_FAILURE;
    }

    std::string logFile = argv[1];
    Logger logger(logFile);

    // Initialize transactions
    std::vector<std::tuple<std::string, double, std::string, int, std::string, std::string, int, std::string>> transactions;
    int transactionCounter = 0;

    for (int i = 2; i < argc; i += 7) {
        double amount = std::stod(argv[i]);
        std::string sourceHost = argv[i + 1];
        int sourcePort = std::stoi(argv[i + 2]);
        std::string sourceAccount = argv[i + 3];
        std::string destHost = argv[i + 4];
        int destPort = std::stoi(argv[i + 5]);
        std::string destAccount = argv[i + 6];

        std::string transactionID = "txn_" + std::to_string(transactionCounter++);
        logger.log("Transaction: $" + std::to_string(amount));
        logger.log("    From: " + sourceHost + ":" + std::to_string(sourcePort) + " account #" + sourceAccount);
        logger.log("    To: " + destHost + ":" + std::to_string(destPort) + " account #" + destAccount);

        transactions.emplace_back(transactionID, amount, sourceHost, sourcePort, sourceAccount, destHost, destPort, destAccount);
    }

    // Start a thread to handle transactions
    std::thread worker(handleTransaction, logFile);

    // Add transactions to the queue and signal the worker
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto& transaction : transactions) {
            transactionQueue.push(transaction);
        }
    }
    cv.notify_one();

    // Signal stop and wait for all transactions to complete
    {
        std::lock_guard<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_one();

    worker.join();

    // Clean up client connections
    for (auto& connection : clientConnections) {
        delete connection.second;
    }

    return EXIT_SUCCESS;
}
