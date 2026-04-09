#ifndef SPECTATOR_H
#define SPECTATOR_H

#include <string>

bool spectator_sender_init(const std::string& host, int port);
void spectator_sender_log(const std::string& line);
void spectator_sender_close();

// Blocking listener for third-node observer mode.
int spectator_run_listener(int port);

#endif // SPECTATOR_H
