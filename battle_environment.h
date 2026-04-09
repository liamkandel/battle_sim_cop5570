#ifndef BATTLE_ENVIRONMENT_H
#define BATTLE_ENVIRONMENT_H

#include "units.h"
#include <string>
#include <vector>

enum WeatherType {
    WEATHER_CLEAR,
    WEATHER_RAIN,
    WEATHER_FOG,
    WEATHER_STORM,
    WEATHER_HEATWAVE
};

enum MapType {
    MAP_PLAINS,
    MAP_URBAN,
    MAP_MOUNTAINS,
    MAP_COASTAL,
    MAP_RUINS
};

struct BattleEnvironment {
    WeatherType weather;
    MapType map;
    int third_party_round;
    int emp_round;
    bool third_party_triggered;
};

BattleEnvironment create_battle_environment();

const char* weather_name(WeatherType weather);
const char* map_name(MapType map);

int apply_environment_damage(const Unit& attacker,
                             const Unit& defender,
                             int base_damage,
                             const BattleEnvironment& env,
                             bool* missed_attack);

bool is_emp_blocked(const Unit& attacker, const BattleEnvironment& env, int round);

void apply_third_party_event(std::vector<Unit>& my_army,
                             std::vector<Unit>& enemy_army,
                             int round,
                             const BattleEnvironment& env,
                             const std::string& player_name,
                             const std::string& opponent_name);

#endif // BATTLE_ENVIRONMENT_H
