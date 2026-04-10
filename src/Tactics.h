// Tactics.h
// Defines the Tactics struct — the three strategic choices a manager makes for their team:
//   1. Formation: How many defenders, midfielders, and forwards to play (e.g. 4-4-2)
//   2. Mentality: How aggressive or cautious the team plays (defensive to attacking)
//   3. Play Style: The overall philosophy (park the bus, counter attack, possession, etc.)
//
// Tactics produce "modifiers" — multipliers that boost or reduce different aspects of play.
// For example, an attacking mentality with direct play style gives a high attack modifier
// but a low defense modifier. These modifiers are used by the match engine to determine
// how likely a team is to score, concede, win possession, etc.

#pragma once
#include "Types.h"

struct Tactics {
    // The three tactical choices. All default to the most neutral options.
    Formation formation = Formation::F_4_4_2;   // Classic balanced formation
    Mentality mentality = Mentality::BALANCED;   // Neither attacking nor defensive
    PlayStyle playStyle = PlayStyle::BALANCED;   // No particular philosophy

    // These five methods return multipliers (usually between 0.5 and 1.5) that the match
    // engine uses to adjust team performance. Each combines the mentality and play style
    // to produce a single number.
    //
    // attackModifier():       How much attacking power your team gets. Higher = more dangerous going forward.
    // defenseModifier():      How solid your defense is. Higher = harder to score against.
    // midfieldModifier():     How dominant you are in midfield. Affected by formation (more MIDs = higher).
    // tempoModifier():        How fast the game is played. Higher = more events per match.
    // shootingRangeModifier(): How willing players are to try long shots. Higher = more attempts from distance.
    float attackModifier() const;
    float defenseModifier() const;
    float midfieldModifier() const;
    float tempoModifier() const;
    float shootingRangeModifier() const;

    // Convenience methods that return the current tactic settings as human-readable strings.
    // Used by the UI to display things like "4-4-2", "Attacking", "Possession".
    std::string formationStr() const { return getFormationInfo(formation).name; }
    std::string mentalityStr() const { return mentalityToString(mentality); }
    std::string playStyleStr() const { return playStyleToString(playStyle); }
};
