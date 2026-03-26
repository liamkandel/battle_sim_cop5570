#include "network.h"
#include "game.h"
#include "battle.h"
#include "protocol.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <csignal>
#include <pthread.h>
#include <unistd.h>

// Global connection for signal handler cleanup
static Connection g_conn;

// Shared state between threads
struct SharedState {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    bool            peer_ready;     // Peer has sent READY
    bool            army_received;  // Peer's army has been received
    ArmyComposition peer_army;      // The peer's army composition
    unsigned int    battle_seed;    // Shared RNG seed
    bool            seed_received;
    bool            disconnected;
};

static SharedState g_state;

// Signal handler for graceful shutdown
void signal_handler(int sig) {
    (void)sig;
    std::cout << "\n  Disconnecting..." << std::endl;
    send_message(g_conn, serialize_disconnect());
    close_connection(g_conn);
    exit(0);
}

// Network receiver thread: reads messages from the peer
void* network_thread(void* arg) {
    Connection* conn = (Connection*)arg;

    while (conn->connected) {
        std::string msg = recv_message(*conn);
        if (msg.empty()) {
            pthread_mutex_lock(&g_state.mutex);
            g_state.disconnected = true;
            pthread_cond_signal(&g_state.cond);
            pthread_mutex_unlock(&g_state.mutex);
            break;
        }

        MessageType type = get_message_type(msg);

        pthread_mutex_lock(&g_state.mutex);
        switch (type) {
            case MSG_READY:
                g_state.peer_ready = true;
                pthread_cond_signal(&g_state.cond);
                break;
            case MSG_ARMY:
                g_state.peer_army = deserialize_army(msg);
                g_state.army_received = true;
                pthread_cond_signal(&g_state.cond);
                break;
            case MSG_SEED:
                g_state.battle_seed = deserialize_seed(msg);
                g_state.seed_received = true;
                pthread_cond_signal(&g_state.cond);
                break;
            case MSG_DISCONNECT:
                std::cout << "\n  Opponent disconnected." << std::endl;
                g_state.disconnected = true;
                pthread_cond_signal(&g_state.cond);
                break;
            default:
                break;
        }
        pthread_mutex_unlock(&g_state.mutex);
    }

    return nullptr;
}

void print_usage(const char* prog) {
    std::cout << "Usage:" << std::endl;
    std::cout << "  Host a game:  " << prog << " --host --port <PORT> [--name <NAME>]" << std::endl;
    std::cout << "  Join a game:  " << prog << " --join <HOSTNAME> <PORT> [--name <NAME>]" << std::endl;
}

int main(int argc, char* argv[]) {
    bool is_host = false;
    bool is_join = false;
    int port = 0;
    const char* hostname = nullptr;
    std::string player_name = "Player";

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--host") == 0) {
            is_host = true;
        } else if (strcmp(argv[i], "--join") == 0) {
            is_join = true;
            if (i + 2 < argc) {
                hostname = argv[++i];
                port = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--name") == 0 && i + 1 < argc) {
            player_name = argv[++i];
        }
    }

    if ((!is_host && !is_join) || port <= 0) {
        print_usage(argv[0]);
        return 1;
    }

    // Setup signal handler
    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    // ==========================================
    // PHASE 1: CONNECT
    // ==========================================
    std::cout << std::endl;
    std::cout << "  ======================================" << std::endl;
    std::cout << "    P2P BATTLE SIMULATOR" << std::endl;
    std::cout << "  ======================================" << std::endl;

    if (is_host) {
        g_conn = host_game(port);
    } else {
        g_conn = join_game(hostname, port);
    }

    if (!g_conn.connected) {
        std::cerr << "Failed to establish connection." << std::endl;
        return 1;
    }

    // Initialize shared state
    pthread_mutex_init(&g_state.mutex, nullptr);
    pthread_cond_init(&g_state.cond, nullptr);
    g_state.peer_ready = false;
    g_state.army_received = false;
    g_state.seed_received = false;
    g_state.disconnected = false;

    // Start network receiver thread
    pthread_t net_tid;
    pthread_create(&net_tid, nullptr, network_thread, &g_conn);

    // ==========================================
    // PHASE 2: ARMY DESIGN
    // ==========================================
    ArmyComposition my_army_comp = design_army();
    display_army(my_army_comp);

    // Send READY signal
    send_message(g_conn, serialize_ready());
    std::cout << std::endl << "  Waiting for opponent to finish their army..." << std::endl;

    // Wait for peer to be ready
    pthread_mutex_lock(&g_state.mutex);
    while (!g_state.peer_ready && !g_state.disconnected) {
        pthread_cond_wait(&g_state.cond, &g_state.mutex);
    }
    pthread_mutex_unlock(&g_state.mutex);

    if (g_state.disconnected) {
        std::cout << "  Opponent disconnected before battle." << std::endl;
        close_connection(g_conn);
        return 1;
    }

    // ==========================================
    // PHASE 3: EXCHANGE ARMIES & SEED
    // ==========================================
    // Send our army
    send_message(g_conn, serialize_army(my_army_comp));

    // Host generates and sends the shared RNG seed
    if (is_host) {
        unsigned int seed = (unsigned int)time(nullptr) ^ (unsigned int)getpid();
        g_state.battle_seed = seed;
        g_state.seed_received = true;
        send_message(g_conn, serialize_seed(seed));
    }

    // Wait for peer's army and seed
    pthread_mutex_lock(&g_state.mutex);
    while ((!g_state.army_received || !g_state.seed_received) && !g_state.disconnected) {
        pthread_cond_wait(&g_state.cond, &g_state.mutex);
    }
    ArmyComposition peer_army_comp = g_state.peer_army;
    unsigned int seed = g_state.battle_seed;
    pthread_mutex_unlock(&g_state.mutex);

    if (g_state.disconnected) {
        std::cout << "  Opponent disconnected before battle." << std::endl;
        close_connection(g_conn);
        return 1;
    }

    display_opponent_army(peer_army_comp);

    // ==========================================
    // PHASE 4: BATTLE
    // ==========================================
    std::vector<Unit> my_units = build_army(my_army_comp);
    std::vector<Unit> enemy_units = build_army(peer_army_comp);

    std::string opponent_name = "Opponent"; // Could be exchanged via protocol

    BattleResult result = run_battle(my_units, enemy_units, seed, player_name, opponent_name);

    // ==========================================
    // PHASE 5: POST-GAME
    // ==========================================
    switch (result) {
        case RESULT_WIN:
            std::cout << std::endl << "  Congratulations, " << player_name << "! You won!" << std::endl;
            break;
        case RESULT_LOSE:
            std::cout << std::endl << "  Better luck next time, " << player_name << "." << std::endl;
            break;
        case RESULT_DRAW:
            std::cout << std::endl << "  It's a draw! Well fought." << std::endl;
            break;
    }

    // Cleanup
    send_message(g_conn, serialize_disconnect());
    close_connection(g_conn);
    pthread_join(net_tid, nullptr);
    pthread_mutex_destroy(&g_state.mutex);
    pthread_cond_destroy(&g_state.cond);

    std::cout << "  Thanks for playing!" << std::endl;
    return 0;
}
