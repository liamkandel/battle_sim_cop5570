#include "network.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

Connection host_game(int port) {
    Connection conn;
    conn.is_host = true;
    conn.connected = false;
    conn.sockfd = -1;

    // Create TCP socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return conn;
    }

    // Allow port reuse
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind to port
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return conn;
    }

    // Listen for 1 connection
    if (listen(listen_fd, 1) < 0) {
        perror("listen");
        close(listen_fd);
        return conn;
    }

    // Get our hostname for display
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    std::cout << "=== Waiting for opponent on " << hostname << ":" << port << " ===" << std::endl;
    std::cout << "Tell your opponent to run: ./battle --join " << hostname << " " << port << std::endl;

    // Accept connection
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept");
        close(listen_fd);
        return conn;
    }

    // Close listening socket, we only need the one connection
    close(listen_fd);

    conn.sockfd = client_fd;
    conn.connected = true;

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    std::cout << "=== Opponent connected from " << client_ip << " ===" << std::endl;

    return conn;
}

Connection join_game(const char* hostname, int port) {
    Connection conn;
    conn.is_host = false;
    conn.connected = false;
    conn.sockfd = -1;

    // Resolve hostname
    struct hostent* server = gethostbyname(hostname);
    if (!server) {
        std::cerr << "Error: could not resolve hostname '" << hostname << "'" << std::endl;
        return conn;
    }

    // Create TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return conn;
    }

    // Connect
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    std::cout << "Connecting to " << hostname << ":" << port << "..." << std::endl;

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return conn;
    }

    conn.sockfd = sockfd;
    conn.connected = true;
    std::cout << "=== Connected to opponent! ===" << std::endl;

    return conn;
}

int send_message(const Connection& conn, const std::string& msg) {
    if (!conn.connected) return -1;

    size_t total = 0;
    size_t len = msg.length();
    const char* buf = msg.c_str();

    while (total < len) {
        ssize_t sent = send(conn.sockfd, buf + total, len - total, 0);
        if (sent <= 0) {
            perror("send");
            return -1;
        }
        total += sent;
    }
    return 0;
}

std::string recv_message(const Connection& conn) {
    if (!conn.connected) return "";

    std::string result;
    char c;

    // Read one character at a time until newline
    while (true) {
        ssize_t n = recv(conn.sockfd, &c, 1, 0);
        if (n <= 0) {
            // Disconnected or error
            return "";
        }
        if (c == '\n') {
            break;
        }
        result += c;
    }
    return result;
}

void close_connection(Connection& conn) {
    if (conn.sockfd >= 0) {
        close(conn.sockfd);
        conn.sockfd = -1;
    }
    conn.connected = false;
}
