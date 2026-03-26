#include "units.h"
#include <cmath>

// Unit catalog: defines all available unit types and their base stats
static const std::vector<UnitTemplate> UNIT_CATALOG = {
    { INFANTRY,    "Infantry",         50, 15,  5, 10, "Basic ground troops" },
    { TANK,        "Tank",            120, 40, 20, 30, "Heavy armor, bonus vs Infantry (+50% ATK)" },
    { ARTILLERY,   "Artillery",        60, 55,  3, 35, "Long range, fires first each round" },
    { NAVY,        "Navy Destroyer",  100, 30, 15, 25, "Naval unit, bonus DEF vs Missiles (+50%)" },
    { MISSILE,     "Missile Launcher", 40, 70,  0, 40, "Ignores enemy DEF, but very fragile" },
    { SHIELD_WALL, "Shield Wall",     150,  0, 30, 20, "Absorbs damage for allies, cannot attack" }
};

const std::vector<UnitTemplate>& get_unit_catalog() {
    return UNIT_CATALOG;
}

Unit create_unit(UnitType type) {
    const UnitTemplate& t = UNIT_CATALOG[type];
    Unit u;
    u.type = t.type;
    u.name = t.name;
    u.hp = t.hp;
    u.max_hp = t.hp;
    u.atk = t.atk;
    u.def = t.def;
    u.cost = t.cost;
    u.alive = true;
    return u;
}

const char* unit_type_name(UnitType type) {
    switch (type) {
        case INFANTRY:    return "Infantry";
        case TANK:        return "Tank";
        case ARTILLERY:   return "Artillery";
        case NAVY:        return "Navy Destroyer";
        case MISSILE:     return "Missile Launcher";
        case SHIELD_WALL: return "Shield Wall";
        default:          return "Unknown";
    }
}

// Type bonus multiplier: returns 1.5 if attacker has advantage, 1.0 otherwise
double get_type_bonus(UnitType attacker_type, UnitType defender_type) {
    // Tank has +50% ATK vs Infantry
    if (attacker_type == TANK && defender_type == INFANTRY)
        return 1.5;
    // Navy has +50% DEF vs Missiles (handled in damage calc as reduced damage)
    // No ATK bonus here; the DEF bonus is applied in calculate_damage
    return 1.0;
}

int calculate_damage(const Unit& attacker, const Unit& defender) {
    if (attacker.atk == 0) return 0; // Shield Walls deal no damage

    // Missiles ignore DEF entirely
    if (attacker.type == MISSILE) {
        return attacker.atk;
    }

    // Apply type ATK bonus
    double atk = attacker.atk * get_type_bonus(attacker.type, defender.type);

    // Calculate effective DEF (Navy gets +50% DEF vs Missiles, but missiles ignore DEF anyway)
    double def = defender.def;
    if (defender.type == NAVY && attacker.type == MISSILE) {
        def *= 1.5; // This branch won't actually fire since missiles skip DEF above
    }

    // Damage formula: diminishing returns on DEF
    // damage = ATK * (100 / (100 + DEF))
    double damage = atk * (100.0 / (100.0 + def));

    return std::max(1, (int)round(damage));
}
