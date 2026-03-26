#include "battle.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <thread>
#include <chrono>
#include <iomanip>

// Helper: count alive units in a vector
static int count_alive(const std::vector<Unit>& army) {
    int count = 0;
    for (const auto& u : army) {
        if (u.alive) count++;
    }
    return count;
}

// Helper: get indices of alive units of a specific type
static std::vector<int> alive_of_type(const std::vector<Unit>& army, UnitType type) {
    std::vector<int> indices;
    for (int i = 0; i < (int)army.size(); i++) {
        if (army[i].alive && army[i].type == type) indices.push_back(i);
    }
    return indices;
}

// Helper: get indices of all alive non-shield units
static std::vector<int> alive_combatants(const std::vector<Unit>& army) {
    std::vector<int> indices;
    for (int i = 0; i < (int)army.size(); i++) {
        if (army[i].alive && army[i].type != SHIELD_WALL) indices.push_back(i);
    }
    return indices;
}

// Helper: get indices of all alive units
static std::vector<int> alive_all(const std::vector<Unit>& army) {
    std::vector<int> indices;
    for (int i = 0; i < (int)army.size(); i++) {
        if (army[i].alive) indices.push_back(i);
    }
    return indices;
}

// Helper: get indices of alive shield walls
static std::vector<int> alive_shields(const std::vector<Unit>& army) {
    std::vector<int> indices;
    for (int i = 0; i < (int)army.size(); i++) {
        if (army[i].alive && army[i].type == SHIELD_WALL) indices.push_back(i);
    }
    return indices;
}

// Print a colored status bar for unit HP
static void print_hp_bar(int hp, int max_hp) {
    int bar_width = 20;
    int filled = (hp * bar_width) / max_hp;
    std::cout << "[";
    for (int i = 0; i < bar_width; i++) {
        std::cout << (i < filled ? "#" : " ");
    }
    std::cout << "] " << hp << "/" << max_hp;
}

// Print the battlefield status
static void print_status(const std::vector<Unit>& my_army,
                          const std::vector<Unit>& enemy_army,
                          const std::string& player_name,
                          const std::string& opponent_name) {
    std::cout << std::endl;
    std::cout << "  " << player_name << "'s forces (" << count_alive(my_army) << " alive):" << std::endl;
    for (const auto& u : my_army) {
        if (!u.alive) continue;
        std::cout << "    " << std::left << std::setw(20) << u.name << " ";
        print_hp_bar(u.hp, u.max_hp);
        std::cout << std::endl;
    }

    std::cout << "  " << opponent_name << "'s forces (" << count_alive(enemy_army) << " alive):" << std::endl;
    for (const auto& u : enemy_army) {
        if (!u.alive) continue;
        std::cout << "    " << std::left << std::setw(20) << u.name << " ";
        print_hp_bar(u.hp, u.max_hp);
        std::cout << std::endl;
    }
}

static void apply_damage_with_shields(std::vector<Unit>& target_army, int target_idx, int damage) {
    // Check if any shield walls can absorb some damage
    auto shields = alive_shields(target_army);
    if (!shields.empty() && target_army[target_idx].type != SHIELD_WALL) {
        // Shield absorbs half the damage
        int absorbed = damage / 2;
        int remaining = damage - absorbed;

        // Distribute absorbed damage to a shield wall
        int shield_idx = shields[rand() % shields.size()];
        target_army[shield_idx].hp -= absorbed;
        if (target_army[shield_idx].hp <= 0) {
            target_army[shield_idx].alive = false;
            std::cout << "      >> " << target_army[shield_idx].name << " shattered absorbing damage!" << std::endl;
        }

        target_army[target_idx].hp -= remaining;
    } else {
        target_army[target_idx].hp -= damage;
    }

    if (target_army[target_idx].hp <= 0) {
        target_army[target_idx].alive = false;
        std::cout << "      >> " << target_army[target_idx].name << " destroyed!" << std::endl;
    }
}

BattleResult run_battle(std::vector<Unit>& my_army,
                        std::vector<Unit>& enemy_army,
                        unsigned int seed,
                        const std::string& player_name,
                        const std::string& opponent_name) {
    srand(seed);

    std::cout << std::endl;
    std::cout << "  ======================================" << std::endl;
    std::cout << "       BATTLE BEGINS!" << std::endl;
    std::cout << "    " << player_name << "  vs  " << opponent_name << std::endl;
    std::cout << "  ======================================" << std::endl;

    int round = 0;
    const int MAX_ROUNDS = 50;

    while (count_alive(my_army) > 0 && count_alive(enemy_army) > 0 && round < MAX_ROUNDS) {
        round++;
        std::cout << std::endl;
        std::cout << "  ---- Round " << round << " ----" << std::endl;

        // Small delay for dramatic effect
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // === PHASE 1: ARTILLERY PRIORITY STRIKE ===
        auto my_artillery = alive_of_type(my_army, ARTILLERY);
        auto enemy_artillery = alive_of_type(enemy_army, ARTILLERY);

        if (!my_artillery.empty() || !enemy_artillery.empty()) {
            std::cout << "    [Artillery Strike Phase]" << std::endl;
        }

        // My artillery fires at random enemy units
        for (int idx : my_artillery) {
            auto targets = alive_all(enemy_army);
            if (targets.empty()) break;
            int target_idx = targets[rand() % targets.size()];
            int dmg = calculate_damage(my_army[idx], enemy_army[target_idx]);
            std::cout << "    " << player_name << "'s Artillery fires at "
                      << enemy_army[target_idx].name << " for " << dmg << " dmg" << std::endl;
            apply_damage_with_shields(enemy_army, target_idx, dmg);
        }

        // Enemy artillery fires at random of my units
        for (int idx : enemy_artillery) {
            auto targets = alive_all(my_army);
            if (targets.empty()) break;
            int target_idx = targets[rand() % targets.size()];
            int dmg = calculate_damage(enemy_army[idx], my_army[target_idx]);
            std::cout << "    " << opponent_name << "'s Artillery fires at "
                      << my_army[target_idx].name << " for " << dmg << " dmg" << std::endl;
            apply_damage_with_shields(my_army, target_idx, dmg);
        }

        // === PHASE 2: MAIN COMBAT ===
        auto my_fighters = alive_combatants(my_army);
        auto enemy_fighters = alive_combatants(enemy_army);

        // Remove artillery from main combat (they already fired)
        my_fighters.erase(
            std::remove_if(my_fighters.begin(), my_fighters.end(),
                           [&](int i) { return my_army[i].type == ARTILLERY; }),
            my_fighters.end());
        enemy_fighters.erase(
            std::remove_if(enemy_fighters.begin(), enemy_fighters.end(),
                           [&](int i) { return enemy_army[i].type == ARTILLERY; }),
            enemy_fighters.end());

        if (!my_fighters.empty() || !enemy_fighters.empty()) {
            std::cout << "    [Main Combat Phase]" << std::endl;
        }

        // My non-artillery combatants attack random enemies
        for (int idx : my_fighters) {
            if (!my_army[idx].alive) continue;
            auto targets = alive_all(enemy_army);
            if (targets.empty()) break;
            int target_idx = targets[rand() % targets.size()];
            int dmg = calculate_damage(my_army[idx], enemy_army[target_idx]);
            if (dmg > 0) {
                std::cout << "    " << player_name << "'s " << my_army[idx].name
                          << " attacks " << enemy_army[target_idx].name
                          << " for " << dmg << " dmg" << std::endl;
                apply_damage_with_shields(enemy_army, target_idx, dmg);
            }
        }

        // Enemy non-artillery combatants attack random of my units
        for (int idx : enemy_fighters) {
            if (!enemy_army[idx].alive) continue;
            auto targets = alive_all(my_army);
            if (targets.empty()) break;
            int target_idx = targets[rand() % targets.size()];
            int dmg = calculate_damage(enemy_army[idx], my_army[target_idx]);
            if (dmg > 0) {
                std::cout << "    " << opponent_name << "'s " << enemy_army[idx].name
                          << " attacks " << my_army[target_idx].name
                          << " for " << dmg << " dmg" << std::endl;
                apply_damage_with_shields(my_army, target_idx, dmg);
            }
        }

        // === STATUS UPDATE ===
        print_status(my_army, enemy_army, player_name, opponent_name);
    }

    // === RESULTS ===
    std::cout << std::endl;
    std::cout << "  ======================================" << std::endl;

    int my_alive = count_alive(my_army);
    int enemy_alive = count_alive(enemy_army);

    BattleResult result;
    if (my_alive > 0 && enemy_alive == 0) {
        std::cout << "    " << player_name << " WINS!" << std::endl;
        result = RESULT_WIN;
    } else if (my_alive == 0 && enemy_alive > 0) {
        std::cout << "    " << opponent_name << " WINS!" << std::endl;
        result = RESULT_LOSE;
    } else {
        std::cout << "    DRAW!" << std::endl;
        result = RESULT_DRAW;
    }

    std::cout << "    Final: " << player_name << " " << my_alive << " units vs "
              << opponent_name << " " << enemy_alive << " units" << std::endl;
    std::cout << "  ======================================" << std::endl;

    return result;
}
