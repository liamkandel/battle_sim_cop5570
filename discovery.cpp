#include "discovery.h"

#include <fstream>
#include <iostream>

Connection join_from_discovery_file(const std::string& nodes_file, int port) {
    std::ifstream in(nodes_file.c_str());
    if (!in) {
        std::cerr << "Could not open discovery file: " << nodes_file << std::endl;
        return Connection{-1, false, false};
    }

    std::string host;
    while (std::getline(in, host)) {
        if (host.empty() || host[0] == '#') continue;
        std::cout << "Discovery: trying " << host << ":" << port << std::endl;
        Connection c = join_game(host.c_str(), port);
        if (c.connected) return c;
    }
    return Connection{-1, false, false};
}
