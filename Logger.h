#ifndef TCPP2_LOGGER_H
#define TCPP2_LOGGER_H

#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>

class Logger {
    std::ofstream logFile;

public:
    Logger(const std::string& filename) {
        logFile.open(filename, std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Unable to open or create log file: " + filename);
        }
    }

    // Move constructor
    Logger(Logger&& other) noexcept : logFile(std::move(other.logFile)) {
        // The actual stream object 'logFile' is moved, no need to manage state manually here
    }

    // Move assignment operator
    Logger& operator=(Logger&& other) noexcept {
        if (this != &other) {
            logFile = std::move(other.logFile);
            // No additional state to manage, std::ofstream handles it
        }
        return *this;
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void log(const std::string& message) {
        if (logFile.is_open()) {
            logFile << message << std::endl;
        }
    }
};

#endif // TCPP2_LOGGER_H
