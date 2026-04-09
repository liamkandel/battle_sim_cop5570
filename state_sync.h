#ifndef STATE_SYNC_H
#define STATE_SYNC_H

#include "units.h"
#include <string>
#include <vector>

unsigned long long hash_battle_state(const std::vector<Unit>& my_army,
                                     const std::vector<Unit>& enemy_army,
                                     int round);

std::string serialize_round_hashes(const std::vector<unsigned long long>& hashes);
std::vector<unsigned long long> deserialize_round_hashes(const std::string& msg);

bool round_hashes_match(const std::vector<unsigned long long>& a,
                        const std::vector<unsigned long long>& b,
                        int* first_mismatch_round);

#endif // STATE_SYNC_H
