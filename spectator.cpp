#include "spectator.h"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

static int g_sock = -1;
static struct sockaddr_in g_addr;
static bool g_ready = false;

bool spectator_sender_init(const std::string& host, int port) {
    g_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_sock < 0) return false;

    struct hostent* server = gethostbyname(host.c_str());
    if (!server) {
        close(g_sock);
        g_sock = -1;
        return false;
    }

    memset(&g_addr, 0, sizeof(g_addr));
    g_addr.sin_family = AF_INET;
    memcpy(&g_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    g_addr.sin_port = htons(port);
    g_ready = true;
    return true;
}

void spectator_sender_log(const std::string& line) {
    if (!g_ready || g_sock < 0) return;
    sendto(g_sock, line.c_str(), line.size(), 0, (struct sockaddr*)&g_addr, sizeof(g_addr));
}

void spectator_sender_close() {
    if (g_sock >= 0) close(g_sock);
    g_sock = -1;
    g_ready = false;
}

int spectator_run_listener(int port) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    std::cout << "Spectator listener running on UDP port " << port << std::endl;
    char buf[2048];
    while (true) {
        ssize_t n = recv(sock, buf, sizeof(buf) - 1, 0);
        if (n <= 0) continue;
        buf[n] = '\0';
        std::cout << "[spectator] " << buf << std::endl;
    }
}
