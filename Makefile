# Compiler and flags
CXX = g++
CPPFLAGS = -std=c++20 -Wall -Werror -pedantic -ggdb -pthread
HDRS = BankServer.h CoordinatorClient.h Logger.h TCPClient.h TCPServer.h
TARGETS = coordinator participant

# Pattern rule to compile .cpp files to .o files
%.o : %.cpp $(HDRS)
	$(CXX) $(CPPFLAGS) -c $< -o $@

# Default target
all: $(TARGETS)

# Rules to create executables
coordinator: BankServer.o CoordinatorClient.o Logger.o TCPClient.o TCPServer.o coordinator.o
	$(CXX) $(CPPFLAGS) $^ -o $@

participant: BankServer.o Logger.o TCPClient.o TCPServer.o participant.o
	$(CXX) $(CPPFLAGS) $^ -o $@

# Clean target
clean:
	rm -f *.o $(TARGETS)

# Phony targets
.PHONY: all clean
