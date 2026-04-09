#include "battle_environment.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>

static int pick_alive_target(const std::vector<Unit>& army) {
    std::vector<int> alive;
    for (int i = 0; i < (int)army.size(); i++) {
        if (army[i].alive) alive.push_back(i);
    }
    if (alive.empty()) return -1;
    return alive[rand() % alive.size()];
}

BattleEnvironment create_battle_environment() {
    BattleEnvironment env;
    env.weather = (WeatherType)(rand() % 5);
    env.map = (MapType)(rand() % 5);
    env.third_party_round = 3 + (rand() % 5); // rounds 3-7
    env.emp_round = 2 + (rand() % 6);         // rounds 2-7
    env.third_party_triggered = false;
    return env;
}

const char* weather_name(WeatherType weather) {
    switch (weather) {
        case WEATHER_CLEAR: return "Clear";
        case WEATHER_RAIN: return "Rain";
        case WEATHER_FOG: return "Fog";
        case WEATHER_STORM: return "Storm";
        case WEATHER_HEATWAVE: return "Heatwave";
        default: return "Unknown";
    }
}

const char* map_name(MapType map) {
    switch (map) {
        case MAP_PLAINS: return "Plains";
        case MAP_URBAN: return "Urban District";
        case MAP_MOUNTAINS: return "Mountain Pass";
        case MAP_COASTAL: return "Coastal Front";
        case MAP_RUINS: return "Ancient Ruins";
        default: return "Unknown";
    }
}

int apply_environment_damage(const Unit& attacker,
                             const Unit& defender,
                             int base_damage,
                             const BattleEnvironment& env,
                             bool* missed_attack) {
    if (missed_attack) *missed_attack = false;
    double mult = 1.0;

    // Weather effects
    if (env.weather == WEATHER_RAIN) {
        if (attacker.type == ARTILLERY || attacker.type == MISSILE) mult *= 0.8;
    } else if (env.weather == WEATHER_FOG) {
        bool low_visibility_unit = attacker.type == ARTILLERY || attacker.type == MISSILE;
        if (low_visibility_unit && (rand() % 100) < 20) {
            if (missed_attack) *missed_attack = true;
            return 0;
        }
    } else if (env.weather == WEATHER_STORM) {
        if (attacker.type == NAVY) mult *= 1.2;
        if (attacker.type == ARTILLERY) mult *= 0.85;
    } else if (env.weather == WEATHER_HEATWAVE) {
        if (attacker.type == INFANTRY) mult *= 1.1;
        if (attacker.type == TANK) mult *= 0.9;
    }

    // Map effects
    if (env.map == MAP_URBAN) {
        if (defender.type == INFANTRY) mult *= 0.85;
    } else if (env.map == MAP_MOUNTAINS) {
        if (attacker.type == ARTILLERY) mult *= 1.2;
        if (attacker.type == TANK) mult *= 0.85;
    } else if (env.map == MAP_COASTAL) {
        if (attacker.type == NAVY) mult *= 1.25;
        if (defender.type == NAVY) mult *= 0.9;
    } else if (env.map == MAP_RUINS) {
        if (attacker.type == MISSILE) mult *= 1.15;
        if (defender.type == SHIELD_WALL) mult *= 0.9;
    }

    int adjusted = (int)(base_damage * mult);
    return std::max(1, adjusted);
}

bool is_emp_blocked(const Unit& attacker, const BattleEnvironment& env, int round) {
    if (round != env.emp_round) return false;
    return attacker.type == ARTILLERY || attacker.type == MISSILE;
}

void apply_third_party_event(std::vector<Unit>& my_army,
                             std::vector<Unit>& enemy_army,
                             int round,
                             const BattleEnvironment& env,
                             const std::string& player_name,
                             const std::string& opponent_name) {
    if (round != env.third_party_round) return;
    int my_target = pick_alive_target(my_army);
    int enemy_target = pick_alive_target(enemy_army);
    if (my_target < 0 && enemy_target < 0) return;

    std::cout << "    [World Event] Rogue third-party drones enter the battlefield!" << std::endl;

    if (my_target >= 0) {
        int dmg = 12 + (rand() % 17);
        my_army[my_target].hp -= dmg;
        std::cout << "      Drones strike " << player_name << "'s " << my_army[my_target].name
                  << " for " << dmg << " damage." << std::endl;
        if (my_army[my_target].hp <= 0) {
            my_army[my_target].alive = false;
            std::cout << "      >> " << my_army[my_target].name << " destroyed by third-party fire!" << std::endl;
        }
    }
    if (enemy_target >= 0) {
        int dmg = 12 + (rand() % 17);
        enemy_army[enemy_target].hp -= dmg;
        std::cout << "      Drones strike " << opponent_name << "'s " << enemy_army[enemy_target].name
                  << " for " << dmg << " damage." << std::endl;
        if (enemy_army[enemy_target].hp <= 0) {
            enemy_army[enemy_target].alive = false;
            std::cout << "      >> " << enemy_army[enemy_target].name << " destroyed by third-party fire!" << std::endl;
        }
    }
}
