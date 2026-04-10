// Tactics.cpp
// Implementation of the five tactical modifier methods.
// Each modifier combines the effects of mentality and play style into a single multiplier.
// The key design: mentality and play style have OPPOSITE effects on attack vs defense.
// If you go more attacking, your attack modifier goes up but your defense modifier goes down.
// This creates the classic football trade-off: push forward and you're vulnerable at the back.

#include "Tactics.h"

// ============================================================================
// ATTACK MODIFIER
// ============================================================================
// How much extra attacking punch your tactics give you.
// Starts at 1.0 (baseline), then gets multiplied by both mentality and play style.
//
// Mentality effects (attack):
//   Very Defensive: 0.70x  (you're barely trying to score)
//   Defensive:      0.85x  (cautious, not committing many bodies forward)
//   Balanced:       1.00x  (baseline)
//   Attacking:      1.15x  (pushing more players into attack)
//   Very Attacking: 1.30x  (everyone forward, last-minute-equaliser energy)
//
// Play style effects (attack):
//   Park the Bus:   0.60x  (10 men behind the ball, almost zero attacking intent)
//   Counter Attack: 0.90x  (only attack on the break, restrained)
//   Balanced:       1.00x  (baseline)
//   Possession:     1.05x  (slight boost — you have the ball more, so slightly more dangerous)
//   Direct:         1.10x  (long balls create some chances quickly)
//   All Out Attack: 1.30x  (maximum attacking intent, throw everything forward)
//
// Example: Very Attacking + All Out Attack = 1.3 * 1.3 = 1.69x attack power.
// Example: Very Defensive + Park the Bus = 0.7 * 0.6 = 0.42x. You are not scoring.
float Tactics::attackModifier() const {
    float mod = 1.0f;
    switch (mentality) {
        case Mentality::VERY_DEFENSIVE: mod *= 0.7f; break;
        case Mentality::DEFENSIVE:      mod *= 0.85f; break;
        case Mentality::BALANCED:       mod *= 1.0f; break;
        case Mentality::ATTACKING:      mod *= 1.15f; break;
        case Mentality::VERY_ATTACKING: mod *= 1.3f; break;
    }
    switch (playStyle) {
        case PlayStyle::PARK_THE_BUS:    mod *= 0.6f; break;
        case PlayStyle::COUNTER_ATTACK:  mod *= 0.9f; break;
        case PlayStyle::BALANCED:        mod *= 1.0f; break;
        case PlayStyle::POSSESSION:      mod *= 1.05f; break;
        case PlayStyle::DIRECT:          mod *= 1.1f; break;
        case PlayStyle::ALL_OUT_ATTACK:  mod *= 1.3f; break;
    }
    return mod;
}

// ============================================================================
// DEFENSE MODIFIER
// ============================================================================
// How much extra defensive solidity your tactics give you.
// This is essentially the INVERSE of the attack modifier — what helps attack hurts defense.
//
// Mentality effects (defense):
//   Very Defensive: 1.30x  (everyone behind the ball, fortress mode)
//   Defensive:      1.15x  (cautious, hard to break down)
//   Balanced:       1.00x  (baseline)
//   Attacking:      0.85x  (gaps appearing at the back)
//   Very Attacking: 0.70x  (wide open at the back, asking for trouble)
//
// Play style effects (defense):
//   Park the Bus:   1.40x  (maximum defensive solidity — the bus is well and truly parked)
//   Counter Attack: 1.15x  (defend first, attack second — solid)
//   Balanced:       1.00x  (baseline)
//   Possession:     0.95x  (slightly risky — high defensive line can be exploited)
//   Direct:         0.90x  (pushing forward leaves gaps)
//   All Out Attack: 0.70x  (what defense? the goalkeeper is practically alone back there)
float Tactics::defenseModifier() const {
    float mod = 1.0f;
    switch (mentality) {
        case Mentality::VERY_DEFENSIVE: mod *= 1.3f; break;
        case Mentality::DEFENSIVE:      mod *= 1.15f; break;
        case Mentality::BALANCED:       mod *= 1.0f; break;
        case Mentality::ATTACKING:      mod *= 0.85f; break;
        case Mentality::VERY_ATTACKING: mod *= 0.7f; break;
    }
    switch (playStyle) {
        case PlayStyle::PARK_THE_BUS:    mod *= 1.4f; break;
        case PlayStyle::COUNTER_ATTACK:  mod *= 1.15f; break;
        case PlayStyle::BALANCED:        mod *= 1.0f; break;
        case PlayStyle::POSSESSION:      mod *= 0.95f; break;
        case PlayStyle::DIRECT:          mod *= 0.9f; break;
        case PlayStyle::ALL_OUT_ATTACK:  mod *= 0.7f; break;
    }
    return mod;
}

// ============================================================================
// MIDFIELD MODIFIER
// ============================================================================
// How dominant your team is in the midfield battle.
// This one is different from attack/defense — it depends on the FORMATION, not just mentality.
//
// The formula: each midfielder above or below 4 (the "standard" number) adds or removes 8%.
//   - A 3-5-2 has 5 midfielders: modifier = 1.0 + (5-4)*0.08 = 1.08x (strong in midfield)
//   - A 4-3-3 has 3 midfielders: modifier = 1.0 + (3-4)*0.08 = 0.92x (weak in midfield)
//   - A 4-4-2 has 4 midfielders: modifier = 1.0 + (4-4)*0.08 = 1.00x (baseline)
//
// If you're playing Possession style, you get an extra 15% boost on top of that,
// because possession football is all about controlling the midfield.
float Tactics::midfieldModifier() const {
    float mod = 1.0f;
    auto info = getFormationInfo(formation);
    // Each extra midfielder above 4 gives +8%, each fewer gives -8%
    mod *= (1.0f + (info.midfielders - 4) * 0.08f);
    // Possession style gets a bonus because it's built around midfield dominance
    if (playStyle == PlayStyle::POSSESSION) mod *= 1.15f;
    return mod;
}

// ============================================================================
// TEMPO MODIFIER
// ============================================================================
// How fast-paced the match is. Higher tempo = more events happen per minute in the match.
// This affects BOTH teams equally (it's the overall pace of the game, not one-sided).
//
// Play style effects:
//   Park the Bus:   0.70x  (slow, boring, time-wasting football. Effective but miserable to watch.)
//   Counter Attack: 1.10x  (quick transitions — fast when attacking, calm when defending)
//   Balanced:       1.00x  (normal pace)
//   Possession:     0.85x  (patient build-up, lots of passing — lower tempo)
//   Direct:         1.20x  (highest tempo — skip the midfield, launch it forward immediately)
//   All Out Attack: 1.15x  (fast and chaotic, like a basketball game with a football)
float Tactics::tempoModifier() const {
    switch (playStyle) {
        case PlayStyle::PARK_THE_BUS:    return 0.7f;
        case PlayStyle::COUNTER_ATTACK:  return 1.1f;
        case PlayStyle::BALANCED:        return 1.0f;
        case PlayStyle::POSSESSION:      return 0.85f;
        case PlayStyle::DIRECT:          return 1.2f;
        case PlayStyle::ALL_OUT_ATTACK:  return 1.15f;
    }
    return 1.0f;  // Fallback (should never reach here)
}

// ============================================================================
// SHOOTING RANGE MODIFIER
// ============================================================================
// How willing your players are to shoot from distance (long shots, speculative efforts).
// Only affected by mentality — a defensive team is less likely to try a 30-yarder,
// while an attacking team will have a crack from anywhere.
//
// Mentality effects:
//   Very Defensive: 0.80x  (only shoot if you absolutely have to)
//   Defensive:      0.90x  (conservative — take safe options)
//   Balanced:       1.00x  (normal shot selection)
//   Attacking:      1.10x  (willing to try from range)
//   Very Attacking: 1.20x  (shooting from everywhere, hoping something goes in)
float Tactics::shootingRangeModifier() const {
    switch (mentality) {
        case Mentality::VERY_DEFENSIVE: return 0.8f;
        case Mentality::DEFENSIVE:      return 0.9f;
        case Mentality::BALANCED:       return 1.0f;
        case Mentality::ATTACKING:      return 1.1f;
        case Mentality::VERY_ATTACKING: return 1.2f;
    }
    return 1.0f;  // Fallback
}
