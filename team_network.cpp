#include "team_network.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>

TeamLobby host_team_game(int port) {
    TeamLobby lobby;
    lobby.listen_fd = -1;
    lobby.is_active = false;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return lobby;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Set non-blocking
    int flags = fcntl(listen_fd, F_GETFL, 0);
    fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return lobby;
    }

    if (listen(listen_fd, 20) < 0) { // Max 20 pending connections
        perror("listen");
        close(listen_fd);
        return lobby;
    }

    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    std::cout << "=== Team Lobby initialized on " << hostname << ":" << port << " ===" << std::endl;
    std::cout << "Waiting for team players to join with: ./battle --join-team " << hostname << " " << port << std::endl;

    lobby.listen_fd = listen_fd;
    lobby.is_active = true;
    return lobby;
}

bool accept_team_client(TeamLobby& lobby, int timeout_ms) {
    if (!lobby.is_active || lobby.listen_fd < 0) return false;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(lobby.listen_fd, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    int ret = select(lobby.listen_fd + 1, &read_fds, nullptr, nullptr, timeout_ms >= 0 ? &timeout : nullptr);
    if (ret > 0 && FD_ISSET(lobby.listen_fd, &read_fds)) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(lobby.listen_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd >= 0) {
            Connection conn;
            conn.sockfd = client_fd;
            conn.is_host = true;
            conn.connected = true;

            // Make child socket blocking for normal recv_message
            int flags = fcntl(client_fd, F_GETFL, 0);
            fcntl(client_fd, F_SETFL, flags & ~O_NONBLOCK);

            lobby.clients.push_back(conn);

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
            std::cout << "=== Client connected from " << client_ip << " ===" << std::endl;
            return true;
        }
    }
    return false;
}

void close_team_lobby(TeamLobby& lobby) {
    if (lobby.listen_fd >= 0) {
        close(lobby.listen_fd);
        lobby.listen_fd = -1;
    }
    for (auto& conn : lobby.clients) {
        close_connection(conn);
    }
    lobby.clients.clear();
    lobby.is_active = false;
}
