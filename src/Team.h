// Team.h
// Defines the Team class — one football club in the league.
// A team has a name, a squad of players, tactical settings, and a budget for transfers.
// It also provides methods to figure out who should start (starting XI) and how strong
// the team is in each area of the pitch (attack, midfield, defense, goalkeeping).

#pragma once
#include "Player.h"
#include "Tactics.h"
#include <vector>
#include <string>

class Team {
public:
    std::string name;               // Team name, e.g. "Alderwick City"
    std::vector<Player> squad;      // All players on the team (usually 18, can vary after transfers)
    Tactics tactics;                // Current tactical setup (formation, mentality, play style)
    int budget = 5000000;           // Transfer budget in pounds. £5M default, varies by team quality.

    // Determines the best 11 players to start based on the current formation.
    // Returns a vector of indices into the squad[] vector.
    // For example, if the formation is 4-4-2, it picks the 1 best GK, 4 best DEFs,
    // 4 best MIDs, and 2 best FWDs (sorted by overall rating within each position).
    // If a position doesn't have enough players, it fills remaining spots with
    // the best available players regardless of position.
    std::vector<int> getStartingEleven() const;

    // These four methods calculate how good the team is in each area of the pitch.
    // They look at ONLY the starting XI players at each position, average their
    // relevant weighted attributes, and return a float (roughly on a 1-20 scale).
    // The match engine uses these numbers to determine who wins possession,
    // who creates chances, who scores goals, and who makes saves.
    float attackStrength() const;        // Average attacking quality of starting forwards
    float midfieldStrength() const;      // Average midfield quality of starting midfielders
    float defenseStrength() const;       // Average defensive quality of starting defenders
    float goalkeeperStrength() const;    // Goalkeeping rating of the starting GK

    // Simple average of all starting XI players' overall ratings.
    // Used for display purposes (e.g. showing team quality on the team selection screen).
    float averageOverall() const;

    // Finds the best player in the squad at a given position (by overall rating).
    // Returns the index into squad[], or -1 if nobody plays that position.
    int bestPlayerAt(Position pos) const;

    // Factory method: creates a brand new team with a randomly generated squad.
    // "quality" determines how good the players are (passed through to Player::generate).
    // Budget is also quality-dependent: better teams get more money (2M + quality * 500k).
    static Team generate(const std::string& teamName, int quality);
};
