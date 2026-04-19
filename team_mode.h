#ifndef TEAM_MODE_H
#define TEAM_MODE_H

#include <string>

// Run the host or join logic for team mode. Returns exit code.
int run_team_mode(bool is_host, int port, const std::string& hostname, const std::string& player_name);

#endif
