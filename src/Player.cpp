// Player.cpp
// Implementation of the Player class. Contains the overall rating calculation,
// market value formula, and the random player generator.

#include "Player.h"
#include <algorithm>  // For std::clamp (restricts a value to a min/max range)
#include <random>     // For random number generation

// Returns a reference to a shared random number generator.
// "static" inside the function means it's created once and reused forever.
// This avoids creating a new RNG every time we need a random number.
static std::mt19937& rng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

// Clamps (restricts) an attribute value to be between 1 and 20.
// If you pass in 25, you get 20. If you pass in -3, you get 1.
// This prevents attributes from going out of the valid 1-20 range
// when we add random variation during player generation.
static int clampAttr(int val) {
    return std::clamp(val, 1, 20);
}

// Generates a random integer between lo and hi (inclusive).
// For example, randRange(1, 6) simulates rolling a six-sided die.
static int randRange(int lo, int hi) {
    return std::uniform_int_distribution<>(lo, hi)(rng());
}

// Calculates the player's overall rating as a weighted average of their attributes.
// Each position weights different attributes differently:
// - Goalkeepers: mostly goalkeeping skill, with some pace/passing/defending
// - Defenders: mostly defending, with pace and passing
// - Midfielders: mostly passing and creativity
// - Forwards: mostly shooting and pace
// The weights are chosen so that a player's "primary" skills matter most.
// The result is divided by 7 because the weights add up to 7 in each case.
int Player::overall() const {
    switch (position) {
        case Position::GK:
            return (goalkeeping * 4 + pace + passing + defending) / 7;
        case Position::DEF:
            return (defending * 3 + pace * 2 + passing + stamina) / 7;
        case Position::MID:
            return (passing * 3 + creativity * 2 + stamina + dribbling) / 7;
        case Position::FWD:
            return (shooting * 3 + pace * 2 + dribbling + creativity) / 7;
    }
    return 10;  // Fallback (should never reach here)
}

// Calculates how much this player would cost on the transfer market.
// The formula: overall^2 * £50,000, then adjusted by age.
// Squaring the overall means the gap between a 15-rated and 20-rated player is HUGE
// (15^2 = 225 vs 20^2 = 400), which mimics real football where elite players cost
// exponentially more than decent ones.
//
// Age multipliers:
//   23 or younger: 130% (young = expensive, lots of future value)
//   24-27: 120% (prime years approaching)
//   28-30: 100% (baseline, peak years)
//   31-33: 60% (declining, fewer years left)
//   34+: 30% (retirement incoming, bargain bin)
//
// The final value is rounded down to the nearest £100,000 for cleanliness.
int Player::marketValue() const {
    int base = overall() * overall() * 50000;
    if (age <= 23) base = base * 130 / 100;
    else if (age <= 27) base = base * 120 / 100;
    else if (age <= 30) base = base * 100 / 100;
    else if (age <= 33) base = base * 60 / 100;
    else base = base * 30 / 100;
    return (base / 100000) * 100000;  // Round down to nearest 100k
}

// Generates a random player with attributes clustered around the given quality level.
// The key idea: each position gets BONUSES to their primary attributes and PENALTIES
// to their irrelevant ones. A forward gets +0 to +4 bonus on shooting, but -5 to -1
// on defending. This creates realistic-feeling players where strikers can't defend
// and goalkeepers can't shoot.
//
// Parameters:
//   pos: What position this player plays
//   quality: Base quality level (1-20). A quality of 15 means most attributes will be near 15.
//   playerName: The name to give this player (generated elsewhere from name pools)
Player Player::generate(Position pos, int quality, const std::string& playerName) {
    Player p;
    p.name = playerName;
    p.age = randRange(18, 35);   // Random age between 18 and 35
    p.position = pos;

    // "spread" is how much random variation each generic attribute gets.
    // quality ± 3 means a quality-12 player will have generic attributes between 9 and 15.
    int spread = 3;
    auto attr = [&]() { return clampAttr(quality + randRange(-spread, spread)); };

    // Stamina is generic for all positions — everyone needs to run.
    p.stamina = attr();

    switch (pos) {
        case Position::GK:
            // Goalkeepers get a BOOST to goalkeeping (quality + 0 to +4)
            // and a big PENALTY to shooting, dribbling, creativity (these don't matter for GKs)
            p.goalkeeping = clampAttr(quality + randRange(0, 4));   // Primary skill: boosted
            p.defending = attr();                                     // Decent — GKs need some
            p.pace = clampAttr(quality + randRange(-4, 0));          // Slightly slower
            p.passing = clampAttr(quality + randRange(-3, 1));       // Can distribute OK
            p.shooting = clampAttr(quality / 2 + randRange(-2, 2)); // Terrible — halved quality
            p.dribbling = clampAttr(quality / 2 + randRange(-2, 2));// Also terrible
            p.creativity = clampAttr(quality / 2 + randRange(-2, 2));
            break;

        case Position::DEF:
            // Defenders get boosted defending, decent pace/passing, poor shooting
            p.defending = clampAttr(quality + randRange(0, 4));      // Primary skill: boosted
            p.pace = attr();                                          // Need to be quick
            p.passing = attr();                                       // Need to pass out from back
            p.shooting = clampAttr(quality + randRange(-5, -1));     // Can't shoot to save their life
            p.dribbling = clampAttr(quality + randRange(-3, 1));     // Meh
            p.goalkeeping = clampAttr(randRange(1, 5));              // Basically zero
            p.creativity = clampAttr(quality + randRange(-4, 0));    // Not their job
            break;

        case Position::MID:
            // Midfielders are the most well-rounded. Good passing and creativity,
            // decent everything else. They're the engine of the team.
            p.passing = clampAttr(quality + randRange(0, 4));        // Primary: great passer
            p.creativity = clampAttr(quality + randRange(0, 3));     // Primary: great vision
            p.dribbling = attr();                                     // Average
            p.pace = attr();                                          // Average
            p.shooting = clampAttr(quality + randRange(-2, 2));      // Can have a go
            p.defending = clampAttr(quality + randRange(-2, 2));     // Can track back
            p.goalkeeping = clampAttr(randRange(1, 5));              // Nope
            break;

        case Position::FWD:
            // Forwards are all about scoring: shooting and pace are boosted,
            // defending is deliberately terrible (they don't track back).
            p.shooting = clampAttr(quality + randRange(0, 4));       // Primary: lethal finisher
            p.pace = clampAttr(quality + randRange(0, 3));           // Primary: fast
            p.dribbling = attr();                                     // Can beat a man
            p.creativity = attr();                                    // Can create something
            p.passing = clampAttr(quality + randRange(-2, 2));       // Average
            p.defending = clampAttr(quality + randRange(-5, -1));    // Absolutely useless defending
            p.goalkeeping = clampAttr(randRange(1, 5));              // Obviously not
            break;
    }

    return p;
}
