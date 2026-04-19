#ifndef TEAM_NETWORK_H
#define TEAM_NETWORK_H

#include "network.h"
#include <vector>

// Lobby structure for tracking connected clients
struct TeamLobby {
    int listen_fd;
    std::vector<Connection> clients;
    bool is_active;
};

// Initialize the host in team mode (starts listening)
TeamLobby host_team_game(int port);

// Wait for or accept new clients. 
// timeout_ms lets it poll without blocking forever.
// Returns true if a new client joined.
bool accept_team_client(TeamLobby& lobby, int timeout_ms);

// Close the entire lobby and all connections
void close_team_lobby(TeamLobby& lobby);

#endif // TEAM_NETWORK_H
