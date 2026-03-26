#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "units.h"
#include <string>
#include <vector>
#include <map>

// Message types
enum MessageType {
    MSG_READY,
    MSG_ARMY,
    MSG_SEED,
    MSG_REMATCH,
    MSG_DISCONNECT,
    MSG_UNKNOWN
};

// Army composition: maps UnitType -> count
typedef std::map<UnitType, int> ArmyComposition;

// Serialize an army composition to a protocol string
// Format: "ARMY|infantry:3,tank:2,missile:1\n"
std::string serialize_army(const ArmyComposition& army);

// Deserialize a protocol string into an army composition
ArmyComposition deserialize_army(const std::string& msg);

// Serialize a READY message
std::string serialize_ready();

// Serialize a SEED message
std::string serialize_seed(unsigned int seed);

// Deserialize a seed from a SEED message
unsigned int deserialize_seed(const std::string& msg);

// Serialize a REMATCH message
std::string serialize_rematch(bool yes);

// Deserialize a REMATCH message
bool deserialize_rematch(const std::string& msg);

// Serialize a DISCONNECT message
std::string serialize_disconnect();

// Determine the type of a received message
MessageType get_message_type(const std::string& msg);

// Build the actual unit list from an army composition
std::vector<Unit> build_army(const ArmyComposition& comp);

#endif // PROTOCOL_H
