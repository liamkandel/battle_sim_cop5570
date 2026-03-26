#ifndef BATTLE_H
#define BATTLE_H

#include "units.h"
#include <vector>
#include <string>

// Result of a battle
enum BattleResult {
    RESULT_WIN,
    RESULT_LOSE,
    RESULT_DRAW
};

// Run the full battle simulation between two armies
// Uses the given RNG seed for deterministic results
// player_name / opponent_name are for display purposes
// Returns the result from the perspective of the "my_army" player
BattleResult run_battle(std::vector<Unit>& my_army,
                        std::vector<Unit>& enemy_army,
                        unsigned int seed,
                        const std::string& player_name,
                        const std::string& opponent_name);

#endif // BATTLE_H
