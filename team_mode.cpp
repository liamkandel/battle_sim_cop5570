#include "team_mode.h"
#include "team_network.h"
#include "game.h"
#include "battle.h"
#include "protocol.h"

#include <iostream>
#include <unistd.h>
#include <sys/select.h>
#include <vector>

void merge_armies(ArmyComposition& dest, const ArmyComposition& src) {
    for (auto const& kv : src) {
        dest[kv.first] += kv.second;
    }
}

int run_team_mode(bool is_host, int port, const std::string& hostname, const std::string& player_name) {
    std::cout << "\n  ======================================" << std::endl;
    std::cout << "    TEAM BATTLE MODE (HOST/JOIN)" << std::endl;
    std::cout << "  ======================================\n" << std::endl;
    
    if (is_host) {
        TeamLobby lobby = host_team_game(port);
        if (!lobby.is_active) return 1;
        
        std::cout << "  Waiting for players to join..." << std::endl;
        std::cout << "  Press ENTER when all players have joined to start." << std::endl;
        
        // Handle STDIN and Lobby connections simultaneously
        int max_fd = lobby.listen_fd;
        
        while (true) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(lobby.listen_fd, &read_fds);
            FD_SET(STDIN_FILENO, &read_fds);
            
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 500000; // 500ms
            
            int ret = select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);
            if (ret > 0) {
                if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                    std::string line;
                    std::getline(std::cin, line);
                    break; // Start game
                }
                if (FD_ISSET(lobby.listen_fd, &read_fds)) {
                    if (accept_team_client(lobby, 0)) {
                        for (auto& c : lobby.clients) {
                            send_message(c, serialize_team_lobby(lobby.clients.size()));
                        }
                    }
                }
            }
            
            // Check for ready msgs from clients (not strictly needed until start, 
            // but we need to eventually read their armies)
        }
        
        std::cout << "  Lobby closed with " << lobby.clients.size() << " players." << std::endl;
        if (lobby.clients.empty()) {
            std::cout << "  Cannot start team game without players." << std::endl;
            close_team_lobby(lobby);
            return 1;
        }
        
        // Broadcast that we are ready for armies
        ArmyComposition teamA_comp;
        ArmyComposition teamB_comp;
        
        std::cout << "\n  Host designing their own army (Team A)..." << std::endl;
        ArmyComposition host_army = design_army();
        merge_armies(teamA_comp, host_army);
        
        std::cout << "\n  Collecting armies from " << lobby.clients.size() << " clients..." << std::endl;
        for (auto& c : lobby.clients) {
            send_message(c, serialize_ready());
        }
        
        // Read armies from all clients
        std::vector<ArmyComposition> client_armies;
        for (auto& c : lobby.clients) {
            bool got_army = false;
            while (c.connected && !got_army) {
                std::string msg = recv_message(c);
                if (msg.empty()) {
                    c.connected = false;
                    break;
                }
                if (get_message_type(msg) == MSG_ARMY) {
                    client_armies.push_back(deserialize_army(msg));
                    got_army = true;
                }
            }
        }
        
        // Randomly assign to teams
        for (size_t i = 0; i < client_armies.size(); i++) {
            if (i % 2 == 0) {
                merge_armies(teamB_comp, client_armies[i]);
            } else {
                merge_armies(teamA_comp, client_armies[i]);
            }
        }
        
        // Broadcast Start
        unsigned int seed = (unsigned int)time(nullptr);
        std::string start_msg = serialize_team_start(seed, teamA_comp, teamB_comp);
        for (auto& c : lobby.clients) {
            if (c.connected) send_message(c, start_msg);
        }
        
        // Run Battle
        std::vector<Unit> my_units = build_army(teamA_comp);
        std::vector<Unit> enemy_units = build_army(teamB_comp);
        
        // Run without waiting for enter (for simplicity) or just interactive
        run_battle(my_units, enemy_units, seed, "Team A", "Team B");
        
        close_team_lobby(lobby);
        
    } else {
        // Client Join
        Connection conn = join_game(hostname.c_str(), port);
        if (!conn.connected) return 1;
        
        std::cout << "  Waiting in lobby..." << std::endl;
        while (conn.connected) {
            std::string msg = recv_message(conn);
            if (msg.empty()) {
                std::cout << "  Disconnected from lobby." << std::endl;
                close_connection(conn);
                return 1;
            }
            if (get_message_type(msg) == MSG_TEAM_LOBBY) {
                int count = deserialize_team_lobby(msg);
                std::cout << "  Lobby updated: " << count << " players joined." << std::endl;
            } else if (get_message_type(msg) == MSG_READY) {
                std::cout << "  Host started game!" << std::endl;
                break;
            }
        }
        
        if (!conn.connected) return 1;
        
        ArmyComposition my_army_comp = design_army();
        send_message(conn, serialize_army(my_army_comp));
        
        std::cout << "  Waiting for other players to submit armies..." << std::endl;
        
        unsigned int seed = 0;
        ArmyComposition teamA_comp;
        ArmyComposition teamB_comp;
        bool started = false;
        
        while (conn.connected) {
            std::string msg = recv_message(conn);
            if (msg.empty()) break;
            
            if (get_message_type(msg) == MSG_TEAM_START) {
                if (deserialize_team_start(msg, seed, teamA_comp, teamB_comp)) {
                    started = true;
                    break;
                }
            }
        }
        
        if (!started) {
            std::cout << "  Failed to start team battle." << std::endl;
            close_connection(conn);
            return 1;
        }
        
        std::vector<Unit> my_units = build_army(teamA_comp);
        std::vector<Unit> enemy_units = build_army(teamB_comp);
        
        run_battle(my_units, enemy_units, seed, "Team A", "Team B");
        
        close_connection(conn);
    }
    
    return 0;
}
