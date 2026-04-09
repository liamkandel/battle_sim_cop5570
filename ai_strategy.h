#ifndef AI_STRATEGY_H
#define AI_STRATEGY_H

#include "units.h"
#include <vector>

// Choose a target index from enemy_army for attacker_idx in my_army.
// Returns -1 if no valid target is available.
int choose_target_index(const std::vector<Unit>& my_army,
                        int attacker_idx,
                        const std::vector<Unit>& enemy_army);

#endif // AI_STRATEGY_H
