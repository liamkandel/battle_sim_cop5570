#include "ai_strategy.h"

#include <algorithm>
#include <cstdlib>

static int pick_random(const std::vector<int>& v) {
    if (v.empty()) return -1;
    return v[rand() % v.size()];
}

int choose_target_index(const std::vector<Unit>& my_army,
                        int attacker_idx,
                        const std::vector<Unit>& enemy_army) {
    if (attacker_idx < 0 || attacker_idx >= (int)my_army.size()) return -1;
    const Unit& attacker = my_army[attacker_idx];

    std::vector<int> alive;
    std::vector<int> artillery_or_missile;
    std::vector<int> tanks;
    std::vector<int> non_shields;

    for (int i = 0; i < (int)enemy_army.size(); i++) {
        if (!enemy_army[i].alive) continue;
        alive.push_back(i);
        if (enemy_army[i].type != SHIELD_WALL) non_shields.push_back(i);
        if (enemy_army[i].type == ARTILLERY || enemy_army[i].type == MISSILE) {
            artillery_or_missile.push_back(i);
        }
        if (enemy_army[i].type == TANK) tanks.push_back(i);
    }
    if (alive.empty()) return -1;

    // Strategy priorities by attacker role.
    if (attacker.type == ARTILLERY) {
        int t = pick_random(artillery_or_missile);
        if (t >= 0) return t;
        t = pick_random(non_shields);
        if (t >= 0) return t;
        return pick_random(alive);
    }
    if (attacker.type == MISSILE) {
        int t = pick_random(tanks);
        if (t >= 0) return t;
    }

    // Default: focus weakest alive non-shield first.
    const std::vector<int>& pool = non_shields.empty() ? alive : non_shields;
    return *std::min_element(pool.begin(), pool.end(),
                             [&](int a, int b) { return enemy_army[a].hp < enemy_army[b].hp; });
}
