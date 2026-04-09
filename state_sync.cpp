#include "state_sync.h"

#include <sstream>
#include <cstdlib>

static unsigned long long mix64(unsigned long long h, unsigned long long v) {
    h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    return h;
}

unsigned long long hash_battle_state(const std::vector<Unit>& my_army,
                                     const std::vector<Unit>& enemy_army,
                                     int round) {
    unsigned long long h = 1469598103934665603ULL;
    h = mix64(h, (unsigned long long)round);

    for (const auto& u : my_army) {
        h = mix64(h, (unsigned long long)u.type);
        h = mix64(h, (unsigned long long)u.hp);
        h = mix64(h, (unsigned long long)u.alive);
    }
    h = mix64(h, 0xABCDEFULL);
    for (const auto& u : enemy_army) {
        h = mix64(h, (unsigned long long)u.type);
        h = mix64(h, (unsigned long long)u.hp);
        h = mix64(h, (unsigned long long)u.alive);
    }
    return h;
}

std::string serialize_round_hashes(const std::vector<unsigned long long>& hashes) {
    std::ostringstream oss;
    oss << "STATEHASH|";
    for (size_t i = 0; i < hashes.size(); i++) {
        if (i) oss << ",";
        oss << hashes[i];
    }
    oss << "\n";
    return oss.str();
}

std::vector<unsigned long long> deserialize_round_hashes(const std::string& msg) {
    std::vector<unsigned long long> out;
    size_t pipe = msg.find('|');
    if (pipe == std::string::npos) return out;

    std::string payload = msg.substr(pipe + 1);
    while (!payload.empty() && (payload.back() == '\n' || payload.back() == '\r')) {
        payload.pop_back();
    }
    std::istringstream iss(payload);
    std::string token;
    while (std::getline(iss, token, ',')) {
        if (token.empty()) continue;
        out.push_back(strtoull(token.c_str(), nullptr, 10));
    }
    return out;
}

bool round_hashes_match(const std::vector<unsigned long long>& a,
                        const std::vector<unsigned long long>& b,
                        int* first_mismatch_round) {
    size_t n = a.size() < b.size() ? a.size() : b.size();
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            if (first_mismatch_round) *first_mismatch_round = (int)i + 1;
            return false;
        }
    }
    if (a.size() != b.size()) {
        if (first_mismatch_round) *first_mismatch_round = (int)n + 1;
        return false;
    }
    if (first_mismatch_round) *first_mismatch_round = -1;
    return true;
}
