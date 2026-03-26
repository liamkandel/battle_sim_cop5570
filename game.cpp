#include "game.h"
#include <iostream>
#include <iomanip>
#include <limits>

// Print a horizontal separator
static void print_separator() {
    std::cout << std::string(60, '-') << std::endl;
}

ArmyComposition design_army() {
    ArmyComposition army;
    int budget = ARMY_BUDGET;
    const auto& catalog = get_unit_catalog();

    std::cout << std::endl;
    print_separator();
    std::cout << "         === ARMY DESIGN PHASE ===" << std::endl;
    std::cout << "  You have " << budget << " points to build your army." << std::endl;
    print_separator();

    while (budget > 0) {
        // Show catalog
        std::cout << std::endl << "  UNIT CATALOG:" << std::endl;
        std::cout << "  " << std::left << std::setw(4) << "#"
                  << std::setw(20) << "Unit"
                  << std::setw(6) << "Cost"
                  << std::setw(6) << "HP"
                  << std::setw(6) << "ATK"
                  << std::setw(6) << "DEF"
                  << "Info" << std::endl;
        print_separator();

        for (int i = 0; i < (int)catalog.size(); i++) {
            const auto& u = catalog[i];
            std::cout << "  " << std::left << std::setw(4) << (i + 1)
                      << std::setw(20) << u.name
                      << std::setw(6) << u.cost
                      << std::setw(6) << u.hp
                      << std::setw(6) << u.atk
                      << std::setw(6) << u.def
                      << u.description << std::endl;
        }

        std::cout << std::endl << "  Budget remaining: " << budget << " pts" << std::endl;

        // Show current army if any units purchased
        bool has_units = false;
        for (auto& kv : army) {
            if (kv.second > 0) { has_units = true; break; }
        }
        if (has_units) {
            std::cout << "  Current army: ";
            bool first = true;
            for (auto& kv : army) {
                if (kv.second <= 0) continue;
                if (!first) std::cout << ", ";
                std::cout << unit_type_name(kv.first) << " x" << kv.second;
                first = false;
            }
            std::cout << std::endl;
        }

        std::cout << std::endl << "  Enter unit # to buy (0 to finish): ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  Invalid input." << std::endl;
            continue;
        }

        if (choice == 0) {
            if (!has_units) {
                std::cout << "  You must buy at least one unit!" << std::endl;
                continue;
            }
            break; // Done building army
        }

        if (choice < 1 || choice > (int)catalog.size()) {
            std::cout << "  Invalid choice." << std::endl;
            continue;
        }

        const auto& selected = catalog[choice - 1];

        // How many?
        int max_affordable = budget / selected.cost;
        if (max_affordable <= 0) {
            std::cout << "  Can't afford " << selected.name << " (" << selected.cost << " pts)." << std::endl;
            continue;
        }

        std::cout << "  How many " << selected.name << "? (max " << max_affordable << "): ";
        int count;
        if (!(std::cin >> count) || count < 1) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  Invalid quantity." << std::endl;
            continue;
        }

        if (count > max_affordable) {
            std::cout << "  Can only afford " << max_affordable << "." << std::endl;
            count = max_affordable;
        }

        army[selected.type] += count;
        budget -= count * selected.cost;
        std::cout << "  Added " << count << "x " << selected.name
                  << " (-" << count * selected.cost << " pts)" << std::endl;
    }

    print_separator();
    std::cout << "  Army finalized! Remaining budget: " << budget << " pts (unspent)" << std::endl;
    print_separator();

    return army;
}

void display_army(const ArmyComposition& army) {
    std::cout << std::endl << "  === YOUR ARMY ===" << std::endl;
    int total_cost = 0;
    for (auto& kv : army) {
        if (kv.second <= 0) continue;
        const auto& catalog = get_unit_catalog();
        const auto& t = catalog[kv.first];
        std::cout << "  " << std::left << std::setw(20) << t.name
                  << "x" << kv.second
                  << "  (HP:" << t.hp << " ATK:" << t.atk << " DEF:" << t.def << ")"
                  << std::endl;
        total_cost += t.cost * kv.second;
    }
    std::cout << "  Total spent: " << total_cost << "/" << ARMY_BUDGET << " pts" << std::endl;
}

void display_opponent_army(const ArmyComposition& army) {
    std::cout << std::endl << "  === OPPONENT'S ARMY ===" << std::endl;
    int total_cost = 0;
    for (auto& kv : army) {
        if (kv.second <= 0) continue;
        const auto& catalog = get_unit_catalog();
        const auto& t = catalog[kv.first];
        std::cout << "  " << std::left << std::setw(20) << t.name
                  << "x" << kv.second
                  << "  (HP:" << t.hp << " ATK:" << t.atk << " DEF:" << t.def << ")"
                  << std::endl;
        total_cost += t.cost * kv.second;
    }
    std::cout << "  Total spent: " << total_cost << " pts" << std::endl;
}
