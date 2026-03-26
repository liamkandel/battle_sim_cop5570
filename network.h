#ifndef NETWORK_H
#define NETWORK_H

#include <string>

// Connection info
struct Connection {
    int sockfd;         // Socket file descriptor
    bool is_host;       // True if we are the host (listener)
    bool connected;     // True if connection established
};

// Host: create a listening socket and wait for a client to connect
// Returns a Connection struct with the accepted client socket
Connection host_game(int port);

// Client: connect to a host at the given hostname and port
Connection join_game(const char* hostname, int port);

// Send a message string over the connection (blocking)
// Returns 0 on success, -1 on error
int send_message(const Connection& conn, const std::string& msg);

// Receive a newline-terminated message from the connection (blocking)
// Returns the message string, or empty string on disconnect/error
std::string recv_message(const Connection& conn);

// Close the connection gracefully
void close_connection(Connection& conn);

#endif // NETWORK_H
