#ifndef UNITS_H
#define UNITS_H

#include <string>
#include <vector>

// Unit type enumeration
enum UnitType {
    INFANTRY,
    TANK,
    ARTILLERY,
    NAVY,
    MISSILE,
    SHIELD_WALL,
    NUM_UNIT_TYPES
};

// Stats for a single unit instance
struct Unit {
    UnitType type;
    std::string name;
    int hp;
    int max_hp;
    int atk;
    int def;
    int cost;
    bool alive;
};

// Unit catalog entry (template for creating units)
struct UnitTemplate {
    UnitType type;
    std::string name;
    int hp;
    int atk;
    int def;
    int cost;
    std::string description;
};

// Get the catalog of all available unit types
const std::vector<UnitTemplate>& get_unit_catalog();

// Create a unit instance from a template
Unit create_unit(UnitType type);

// Get the name string for a unit type
const char* unit_type_name(UnitType type);

// Calculate damage from attacker to defender, applying type bonuses
// Returns the damage dealt
int calculate_damage(const Unit& attacker, const Unit& defender);

// Check if attacker has a type bonus against defender
double get_type_bonus(UnitType attacker_type, UnitType defender_type);

#endif // UNITS_H
