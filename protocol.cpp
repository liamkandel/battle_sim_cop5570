#include "protocol.h"
#include <sstream>
#include <cstdlib>
#include <algorithm>

// Map between unit type names (lowercase) and UnitType enum
static const char* TYPE_NAMES[] = {
    "infantry", "tank", "artillery", "navy", "missile", "shield_wall"
};

static UnitType name_to_type(const std::string& name) {
    for (int i = 0; i < NUM_UNIT_TYPES; i++) {
        if (name == TYPE_NAMES[i]) return (UnitType)i;
    }
    return INFANTRY; // fallback
}

static const char* type_to_name(UnitType t) {
    if (t >= 0 && t < NUM_UNIT_TYPES) return TYPE_NAMES[t];
    return "unknown";
}

// --- Serialization ---

std::string serialize_army(const ArmyComposition& army) {
    std::ostringstream oss;
    oss << "ARMY|";
    bool first = true;
    for (auto it = army.begin(); it != army.end(); ++it) {
        if (it->second <= 0) continue;
        if (!first) oss << ",";
        oss << type_to_name(it->first) << ":" << it->second;
        first = false;
    }
    oss << "\n";
    return oss.str();
}

ArmyComposition deserialize_army(const std::string& msg) {
    ArmyComposition army;
    // Expected format: "ARMY|infantry:3,tank:2,missile:1"
    size_t pipe = msg.find('|');
    if (pipe == std::string::npos) return army;

    std::string payload = msg.substr(pipe + 1);
    // Remove trailing newline/whitespace
    while (!payload.empty() && (payload.back() == '\n' || payload.back() == '\r'))
        payload.pop_back();

    std::istringstream iss(payload);
    std::string token;
    while (std::getline(iss, token, ',')) {
        size_t colon = token.find(':');
        if (colon == std::string::npos) continue;
        std::string name = token.substr(0, colon);
        int count = atoi(token.substr(colon + 1).c_str());
        army[name_to_type(name)] = count;
    }
    return army;
}

std::string serialize_ready() {
    return "READY\n";
}

std::string serialize_seed(unsigned int seed) {
    std::ostringstream oss;
    oss << "SEED|" << seed << "\n";
    return oss.str();
}

unsigned int deserialize_seed(const std::string& msg) {
    size_t pipe = msg.find('|');
    if (pipe == std::string::npos) return 0;
    return (unsigned int)strtoul(msg.substr(pipe + 1).c_str(), nullptr, 10);
}

std::string serialize_rematch(bool yes) {
    return yes ? "REMATCH|yes\n" : "REMATCH|no\n";
}

bool deserialize_rematch(const std::string& msg) {
    return msg.find("yes") != std::string::npos;
}

std::string serialize_disconnect() {
    return "DISCONNECT\n";
}

MessageType get_message_type(const std::string& msg) {
    if (msg.substr(0, 5) == "READY") return MSG_READY;
    if (msg.substr(0, 4) == "ARMY") return MSG_ARMY;
    if (msg.substr(0, 4) == "SEED") return MSG_SEED;
    if (msg.substr(0, 7) == "REMATCH") return MSG_REMATCH;
    if (msg.substr(0, 10) == "DISCONNECT") return MSG_DISCONNECT;
    return MSG_UNKNOWN;
}

std::vector<Unit> build_army(const ArmyComposition& comp) {
    std::vector<Unit> units;
    for (auto it = comp.begin(); it != comp.end(); ++it) {
        for (int i = 0; i < it->second; i++) {
            units.push_back(create_unit(it->first));
        }
    }
    return units;
}
