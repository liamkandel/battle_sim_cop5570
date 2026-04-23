#include "team_mode.h"
#include "team_network.h"
#include "game.h"
#include "battle.h"
#include "protocol.h"
#include "battle_environment.h"

#include <iostream>
#include <unistd.h>
#include <sys/select.h>
#include <vector>
#include <ctime>
#include <cstdlib>

void merge_armies(ArmyComposition& dest, const ArmyComposition& src) {
    for (auto const& kv : src) {
        dest[kv.first] += kv.second;
    }
}

static void show_intel_preview(unsigned int seed) {
    srand(seed);
    BattleEnvironment env = create_battle_environment();
    std::cout << std::endl;
    std::cout << "  +------------------------------------------------------+" << std::endl;
    std::cout << "  |      BATTLEFIELD INTEL  --  Plan Your Army!          |" << std::endl;
    std::cout << "  +------------------------------------------------------+" << std::endl;
    std::cout << "  | Weather : " << weather_name(env.weather)               << std::endl;
    std::cout << "  | Terrain : " << map_name(env.map)                       << std::endl;
    std::cout << "  | 3rd-party strike on round : " << env.third_party_round << std::endl;
    std::cout << "  | EMP event on round        : " << env.emp_round         << std::endl;
    std::cout << "  +------------------------------------------------------+" << std::endl;
    std::cout << "  Use this intel to choose the best army for the conditions!" << std::endl;
    std::cout << std::endl;
}

int run_team_mode(bool is_host, int port, const std::string& hostname, const std::string& player_name) {
    std::cout << std::endl;
    std::cout << "  +============================================+" << std::endl;
    std::cout << "  |       TEAM BATTLE SIMULATOR  v1.0          |" << std::endl;
    std::cout << "  |     Multiple Players  --  Two Teams        |" << std::endl;
    std::cout << "  +============================================+" << std::endl;

    if (is_host) {
        // ── HOST PATH ────────────────────────────────────────────────────
        TeamLobby lobby = host_team_game(port);
        if (!lobby.is_active) return 1;

        std::cout << std::endl;
        std::cout << "  You are the HOST -- " << player_name << std::endl;
        std::cout << "  Waiting for players to join on port " << port << "..." << std::endl;
        std::cout << "  Press ENTER when all players have joined to start." << std::endl;
        std::cout << std::endl;

        // Lobby loop: watch both the listen socket and stdin with select().
        int max_fd = lobby.listen_fd;
        while (true) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(lobby.listen_fd, &read_fds);
            FD_SET(STDIN_FILENO, &read_fds);

            struct timeval tv;
            tv.tv_sec  = 0;
            tv.tv_usec = 500000; // 500 ms

            int ret = select(max_fd + 1, &read_fds, nullptr, nullptr, &tv);
            if (ret > 0) {
                if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                    std::string line;
                    std::getline(std::cin, line);
                    if (lobby.clients.empty()) {
                        std::cout << "  No players have joined yet — keep waiting." << std::endl;
                    } else {
                        break;
                    }
                }
                if (FD_ISSET(lobby.listen_fd, &read_fds)) {
                    if (accept_team_client(lobby, 0)) {
                        size_t n = lobby.clients.size();
                        // First joiner → Team B, second → Team A, alternating
                        std::string assigned = (n % 2 == 0) ? "Team B" : "Team A";
                        std::cout << "  Player #" << n << " connected!"
                                  << "  Assigned to: " << assigned << std::endl;
                        for (auto& c : lobby.clients)
                            send_message(c, serialize_team_lobby((int)n));
                    }
                }
            }
        }

        std::cout << std::endl;
        std::cout << "  Lobby closed — " << lobby.clients.size() << " player(s) joined." << std::endl;

        // ── Generate seed early so everyone gets the same intel ─────────
        unsigned int seed = (unsigned int)time(nullptr);

        // Send seed first so clients can show the intel preview
        for (auto& c : lobby.clients)
            send_message(c, serialize_seed(seed));

        // Show intel to host
        show_intel_preview(seed);

        // Signal clients to start designing their armies
        for (auto& c : lobby.clients)
            send_message(c, serialize_ready());

        // ── Collect armies ───────────────────────────────────────────────
        ArmyComposition teamA_comp;
        ArmyComposition teamB_comp;

        std::cout << "  +--------------------------------------------+" << std::endl;
        std::cout << "  | You are: " << player_name << std::endl;
        std::cout << "  | Your Team: TEAM A  (Host)" << std::endl;
        std::cout << "  +--------------------------------------------+" << std::endl;
        std::cout << "  Design your army!" << std::endl;

        ArmyComposition host_army = design_army();
        merge_armies(teamA_comp, host_army);

        std::cout << std::endl;
        std::cout << "  Waiting for " << lobby.clients.size()
                  << " client(s) to submit armies..." << std::endl;

        std::vector<ArmyComposition> client_armies;
        for (size_t i = 0; i < lobby.clients.size(); i++) {
            auto& c = lobby.clients[i];
            bool got = false;
            while (c.connected && !got) {
                std::string msg = recv_message(c);
                if (msg.empty()) { c.connected = false; break; }
                if (get_message_type(msg) == MSG_ARMY) {
                    client_armies.push_back(deserialize_army(msg));
                    got = true;
                    std::cout << "  Army received from Player #" << (i + 1) << std::endl;
                }
            }
            if (!got) client_armies.push_back(ArmyComposition{});
        }

        // ── Assign teams (alternating) and build rosters ─────────────────
        // Host → Team A. Client 0 → Team B, Client 1 → Team A, etc.
        std::vector<char> client_teams(client_armies.size());
        for (size_t i = 0; i < client_armies.size(); i++) {
            if (i % 2 == 0) {
                merge_armies(teamB_comp, client_armies[i]);
                client_teams[i] = 'B';
            } else {
                merge_armies(teamA_comp, client_armies[i]);
                client_teams[i] = 'A';
            }
        }

        std::cout << std::endl;
        std::cout << "  --- Team Assignments ---" << std::endl;
        std::cout << "  Team A: " << player_name << " (host)" << std::endl;
        for (size_t i = 0; i < client_teams.size(); i++)
            std::cout << "  Team " << client_teams[i]
                      << ": Player #" << (i + 1) << std::endl;
        std::cout << "  ------------------------" << std::endl;

        // ── Send each client their personalised TEAMSTART ────────────────
        for (size_t i = 0; i < lobby.clients.size(); i++) {
            if (lobby.clients[i].connected) {
                char t = (i < client_teams.size()) ? client_teams[i] : 'A';
                send_message(lobby.clients[i],
                             serialize_team_start(seed, teamA_comp, teamB_comp, t));
            }
        }

        // ── Run battle ───────────────────────────────────────────────────
        std::vector<Unit> my_units    = build_army(teamA_comp);
        std::vector<Unit> enemy_units = build_army(teamB_comp);
        std::string my_label = "TEAM A (" + player_name + ")";
        run_battle(my_units, enemy_units, seed, my_label, "TEAM B");

        close_team_lobby(lobby);

    } else {
        // ── CLIENT PATH ──────────────────────────────────────────────────
        Connection conn = join_game(hostname.c_str(), port);
        if (!conn.connected) return 1;

        std::cout << std::endl;
        std::cout << "  Connected to host! Welcome, " << player_name << "!" << std::endl;
        std::cout << "  Waiting in lobby for host to start..." << std::endl;

        int my_join_number = -1;
        unsigned int preview_seed = 0;
        bool has_seed = false;

        while (conn.connected) {
            std::string msg = recv_message(conn);
            if (msg.empty()) {
                std::cout << "  Disconnected from lobby." << std::endl;
                close_connection(conn);
                return 1;
            }
            MessageType mtype = get_message_type(msg);
            if (mtype == MSG_TEAM_LOBBY) {
                int count = deserialize_team_lobby(msg);
                if (my_join_number < 0) my_join_number = count;
                std::cout << "  [Lobby] " << count << " player(s) now in the lobby." << std::endl;
            } else if (mtype == MSG_SEED) {
                preview_seed = deserialize_seed(msg);
                has_seed = true;
            } else if (mtype == MSG_READY) {
                std::cout << "  Host has started the game!" << std::endl;
                break;
            }
        }
        if (!conn.connected) return 1;

        // Determine team from join order (mirrors host-side assignment)
        int join_idx = (my_join_number > 0) ? (my_join_number - 1) : 0;
        std::string my_team_str = (join_idx % 2 == 0) ? "Team B" : "Team A";

        std::cout << std::endl;
        std::cout << "  +--------------------------------------------+" << std::endl;
        std::cout << "  | You are: " << player_name << std::endl;
        std::cout << "  | Your Team: " << my_team_str << std::endl;
        std::cout << "  +--------------------------------------------+" << std::endl;

        if (has_seed)
            show_intel_preview(preview_seed);

        std::cout << "  Design your army for " << my_team_str << "!" << std::endl;

        ArmyComposition my_army_comp = design_army();
        send_message(conn, serialize_army(my_army_comp));

        std::cout << "  Army submitted! Waiting for all players..." << std::endl;

        unsigned int seed = 0;
        ArmyComposition teamA_comp;
        ArmyComposition teamB_comp;
        char my_team = 'A';
        bool started = false;

        while (conn.connected) {
            std::string msg = recv_message(conn);
            if (msg.empty()) break;
            if (get_message_type(msg) == MSG_TEAM_START) {
                if (deserialize_team_start(msg, seed, teamA_comp, teamB_comp, my_team)) {
                    started = true;
                    break;
                }
            }
        }

        if (!started) {
            std::cout << "  Failed to receive battle start." << std::endl;
            close_connection(conn);
            return 1;
        }

        std::cout << std::endl;
        std::cout << "  ======================================================" << std::endl;
        std::cout << "                   THE BATTLE IS STARTING!" << std::endl;
        std::cout << "  ======================================================" << std::endl;
        std::cout << "  You are: " << player_name
                  << "  on  Team " << my_team << std::endl;

        std::vector<Unit> my_units    = build_army(my_team == 'A' ? teamA_comp : teamB_comp);
        std::vector<Unit> enemy_units = build_army(my_team == 'A' ? teamB_comp : teamA_comp);
        std::string my_label    = std::string("TEAM ") + my_team + " (" + player_name + ")";
        std::string enemy_label = (my_team == 'A') ? "TEAM B" : "TEAM A";

        run_battle(my_units, enemy_units, seed, my_label, enemy_label);

        close_connection(conn);
    }

    return 0;
}
