// Team.cpp
// Implementation of the Team class. This is where the squad selection logic lives,
// along with the strength calculations and team generation.

#include "Team.h"
#include "DataGen.h"
#include <algorithm>
#include <numeric>
#include <random>

// Shared random number generator for this file. Created once, reused forever.
static std::mt19937& rng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

// ============================================================================
// GET STARTING ELEVEN
// ============================================================================
// Figures out the best 11 players to start based on the current formation.
//
// Algorithm:
//   1. Build a list of all players with their index, overall rating, and position.
//   2. For each position slot in the formation (e.g. 4 DEFs in a 4-4-2),
//      pick the highest-rated available players at that position.
//   3. If we still don't have 11 (e.g. the team only has 3 DEFs but the formation
//      needs 4), fill remaining spots with the best remaining players regardless
//      of their position. This prevents crashes when squads are weirdly composed.
//
// Returns: Vector of indices into squad[]. Always returns up to 11 indices
//          (or fewer if the squad has fewer than 11 players, which shouldn't happen).
std::vector<int> Team::getStartingEleven() const {
    auto info = getFormationInfo(tactics.formation);
    std::vector<int> eleven;

    // Candidate struct: tracks each player's squad index, overall, and position
    struct Candidate { int idx; int ovr; Position pos; };
    std::vector<Candidate> candidates;
    for (int i = 0; i < (int)squad.size(); i++) {
        candidates.push_back({i, squad[i].overall(), squad[i].position});
    }

    // Lambda that picks the N best players at a given position who haven't already been selected.
    // It filters by position, sorts by overall (descending), and takes the top N.
    auto pickBest = [&](Position pos, int count) {
        std::vector<Candidate> pool;
        for (auto& c : candidates) {
            if (c.pos == pos) {
                // Check this player isn't already in the starting XI
                bool taken = false;
                for (int idx : eleven) if (idx == c.idx) taken = true;
                if (!taken) pool.push_back(c);
            }
        }
        // Sort by overall rating, highest first
        std::sort(pool.begin(), pool.end(), [](auto& a, auto& b) { return a.ovr > b.ovr; });
        // Take the top 'count' players
        for (int i = 0; i < count && i < (int)pool.size(); i++) {
            eleven.push_back(pool[i].idx);
        }
    };

    // Pick players for each position according to the formation
    pickBest(Position::GK, 1);                  // Always exactly 1 goalkeeper
    pickBest(Position::DEF, info.defenders);     // e.g. 4 defenders in a 4-4-2
    pickBest(Position::MID, info.midfielders);   // e.g. 4 midfielders in a 4-4-2
    pickBest(Position::FWD, info.forwards);      // e.g. 2 forwards in a 4-4-2

    // Safety net: if we still don't have 11 players (e.g. missing a position),
    // fill remaining spots with the best available players regardless of position.
    // This prevents weird edge cases after lots of transfers.
    while ((int)eleven.size() < 11 && (int)eleven.size() < (int)squad.size()) {
        int bestIdx = -1;
        int bestOvr = -1;
        for (int i = 0; i < (int)squad.size(); i++) {
            bool taken = false;
            for (int idx : eleven) if (idx == i) taken = true;
            if (!taken && squad[i].overall() > bestOvr) {
                bestOvr = squad[i].overall();
                bestIdx = i;
            }
        }
        if (bestIdx >= 0) eleven.push_back(bestIdx);
        else break;  // No more players available at all
    }

    return eleven;
}

// ============================================================================
// ATTACK STRENGTH
// ============================================================================
// Calculates the average attacking quality of the starting forwards.
// Uses the same weighted formula as Player::overall() for FWD:
//   (shooting*3 + pace*2 + dribbling + creativity) / 7
//
// If no forwards are in the starting XI (somehow), returns 10.0 as a safe fallback.
float Team::attackStrength() const {
    auto xi = getStartingEleven();
    float total = 0;
    int count = 0;
    for (int idx : xi) {
        auto& p = squad[idx];
        if (p.position == Position::FWD) {
            total += (p.shooting * 3 + p.pace * 2 + p.dribbling + p.creativity) / 7.0f;
            count++;
        }
    }
    return count > 0 ? total / count : 10.0f;
}

// ============================================================================
// MIDFIELD STRENGTH
// ============================================================================
// Same idea as attackStrength(), but for midfielders.
// Uses the MID weighting: (passing*3 + creativity*2 + stamina + dribbling) / 7
float Team::midfieldStrength() const {
    auto xi = getStartingEleven();
    float total = 0;
    int count = 0;
    for (int idx : xi) {
        auto& p = squad[idx];
        if (p.position == Position::MID) {
            total += (p.passing * 3 + p.creativity * 2 + p.stamina + p.dribbling) / 7.0f;
            count++;
        }
    }
    return count > 0 ? total / count : 10.0f;
}

// ============================================================================
// DEFENSE STRENGTH
// ============================================================================
// Same idea, but for defenders.
// Uses the DEF weighting: (defending*3 + pace*2 + passing + stamina) / 7
float Team::defenseStrength() const {
    auto xi = getStartingEleven();
    float total = 0;
    int count = 0;
    for (int idx : xi) {
        auto& p = squad[idx];
        if (p.position == Position::DEF) {
            total += (p.defending * 3 + p.pace * 2 + p.passing + p.stamina) / 7.0f;
            count++;
        }
    }
    return count > 0 ? total / count : 10.0f;
}

// ============================================================================
// GOALKEEPER STRENGTH
// ============================================================================
// Returns the raw goalkeeping attribute of the starting goalkeeper.
// Unlike the other strengths, this isn't an average — there's only one GK.
// If somehow there's no GK in the starting XI, returns 5.0 (barely functional).
float Team::goalkeeperStrength() const {
    auto xi = getStartingEleven();
    for (int idx : xi) {
        if (squad[idx].position == Position::GK) {
            return (float)squad[idx].goalkeeping;
        }
    }
    return 5.0f;  // No goalkeeper? You're in trouble.
}

// ============================================================================
// AVERAGE OVERALL
// ============================================================================
// Simple average of all starting XI players' overall ratings.
// Used for display only — the team selection screen shows this so the player
// can compare teams before choosing one.
float Team::averageOverall() const {
    auto xi = getStartingEleven();
    if (xi.empty()) return 0;
    float total = 0;
    for (int idx : xi) total += squad[idx].overall();
    return total / (float)xi.size();
}

// ============================================================================
// BEST PLAYER AT POSITION
// ============================================================================
// Scans the entire squad (not just starters) and returns the index of the
// highest-rated player at the given position. Returns -1 if nobody plays that position.
int Team::bestPlayerAt(Position pos) const {
    int bestIdx = -1;
    int bestOvr = -1;
    for (int i = 0; i < (int)squad.size(); i++) {
        if (squad[i].position == pos && squad[i].overall() > bestOvr) {
            bestOvr = squad[i].overall();
            bestIdx = i;
        }
    }
    return bestIdx;
}

// ============================================================================
// GENERATE TEAM
// ============================================================================
// Factory method that creates a brand new team from scratch.
// Steps:
//   1. Set the team name.
//   2. Calculate the budget: £2M base + £500k per quality point.
//      So a quality-16 team gets £2M + £8M = £10M, while a quality-8 team gets £6M.
//   3. Generate 18 players: 2 GKs, 6 DEFs, 6 MIDs, 4 FWDs.
//      Each player's quality is the team quality ±2 (with some random variation),
//      clamped to 3-19 so nobody is absurdly bad or impossibly good.
Team Team::generate(const std::string& teamName, int quality) {
    Team t;
    t.name = teamName;
    t.budget = 2000000 + quality * 500000;  // Better teams get more money

    // Helper lambda: generates 'count' players at the given position
    auto gen = [&](Position pos, int count) {
        for (int i = 0; i < count; i++) {
            // Each player's quality varies ±2 from the team quality
            int q = quality + std::uniform_int_distribution<>(-2, 2)(rng());
            q = std::clamp(q, 3, 19);  // Keep it within sensible bounds
            std::string pname = DataGen::generatePlayerName(rng());
            t.squad.push_back(Player::generate(pos, q, pname));
        }
    };

    // Standard squad composition: 2 goalkeepers, 6 each of DEF and MID, 4 forwards.
    // That's 18 total — enough for a starting XI of 11 plus 7 subs/backups.
    gen(Position::GK, 2);
    gen(Position::DEF, 6);
    gen(Position::MID, 6);
    gen(Position::FWD, 4);

    return t;
}
