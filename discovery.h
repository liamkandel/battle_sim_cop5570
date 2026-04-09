#ifndef DISCOVERY_H
#define DISCOVERY_H

#include "network.h"
#include <string>

// Attempt to connect by trying hostnames in a file (one per line).
// Returns first successful connection; otherwise connected=false.
Connection join_from_discovery_file(const std::string& nodes_file, int port);

#endif // DISCOVERY_H
