# Distributed Banking System

## Overview

This project implements a distributed banking system using TCP for communication. The system includes a server to handle account transactions and a coordinator to manage transactions between accounts. It ensures thread safety and supports concurrent transactions.

## Features

1. **Logger Class**:
  - A Logger class is implemented to help both client and server log important events and errors.
2. **TCP Server and Client**:
  - The project includes BankServer and CoordinatorClient classes that extend TCPServer and TCPClient respectively, designed to handle specific functionalities required for the banking system.

## EXTRA CREDITS

3. **Concurrent Multiple Users Handling**:
  - The system can handle multiple users simultaneously, ensuring independent and correct transaction processing. It avoids duplicate connections for the same port at one time.
4. **Multithreading for Concurrent Transactions**:
  - Supports multithreading, allowing a single coordinator to handle multiple transactions or multiple coordinators to handle transactions concurrently, ensuring transaction isolation and correctness.

## How to Run

### Running the Bank Server

```sh
./bankserver <port> <accounts_file> <log_file>
./bankserver 10101 accounts.txt bankserver_log.txt
Running the Coordinator Client
./coordinator <log_file> <initial_amount> <initial_source_host> <initial_source_port> <initial_source_account> <initial_dest_host> <initial_dest_port> <initial_dest_account> [<amount> <participant_host> <participant_port> <participant_source_account> <participant_dest_host> <participant_dest_port> <participant_dest_account> ...]
For multiple users on one coordinator:
./coordinator log.txt 200.00 localhost 10101 account1 localhost 10102 account2 300.00 localhost 10102 account2 localhost 10103 account3
For multiple users on multiple coordinators:
./coordinator log1.txt 200.00 localhost 10101 account1 localhost 10102 account2 &
./coordinator log2.txt 300.00 localhost 10102 account2 localhost 10103 account3 &
