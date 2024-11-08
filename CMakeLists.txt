cmake_minimum_required(VERSION 3.26)
project(TCPP2)

set(CMAKE_CXX_STANDARD 17)

add_executable(TCPP2 coordinator.cpp
        participant.cpp
        TCPClient.cpp
        TCPServer.cpp
        BankServer.cpp
        BankServer.h
        CoordinatorClient.cpp
        CoordinatorClient.h
        Logger.cpp
        Logger.h)
