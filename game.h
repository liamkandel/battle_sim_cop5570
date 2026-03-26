#ifndef GAME_H
#define GAME_H

#include "units.h"
#include "protocol.h"
#include <vector>

// Total budget each player gets to build their army
#define ARMY_BUDGET 200

// Display the unit catalog and let the player build an army
// Returns the army composition (UnitType -> count)
ArmyComposition design_army();

// Display a summary of the player's army
void display_army(const ArmyComposition& army);

// Display the opponent's army
void display_opponent_army(const ArmyComposition& army);

#endif // GAME_H
