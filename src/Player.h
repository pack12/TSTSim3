// Player.h
// Defines the Player class — one individual footballer in the game.
// Each player has a name, age, position, eight skill attributes (rated 1-20),
// and season statistics (goals, assists, appearances).

#pragma once
#include "Types.h"
#include <string>

class Player {
public:
    // ----- IDENTITY -----
    std::string name;                       // Full name, e.g. "James Smith"
    int age = 25;                           // Age in years. Players retire after 36.
    Position position = Position::MID;      // What position they play (GK, DEF, MID, FWD)

    // ----- ATTRIBUTES (all rated 1-20, Football Manager style) -----
    // Higher is better. A 20 is world class, a 1 is "why are you on this pitch".
    int pace = 10;          // How fast the player can run
    int passing = 10;       // How accurately they can pass the ball
    int shooting = 10;      // How well they can shoot (finishing, long shots)
    int defending = 10;     // How good they are at tackling and intercepting
    int dribbling = 10;     // How well they can run with the ball at their feet
    int goalkeeping = 10;   // How good they are at stopping shots (only matters for GKs)
    int stamina = 10;       // How long they can run before getting tired
    int creativity = 10;    // How good they are at creating chances (vision, through balls)

    // ----- SEASON STATISTICS -----
    // These reset to 0 at the start of each new season.
    int goals = 0;          // Goals scored this season
    int assists = 0;        // Assists (passes that led directly to a goal) this season
    int appearances = 0;    // Number of matches played this season

    // Calculates a single "overall" rating (1-20) by weighting the attributes
    // differently depending on position. A goalkeeper's overall is mostly about
    // their goalkeeping stat, while a forward's overall is mostly about shooting and pace.
    int overall() const;

    // Estimates the player's transfer value in pounds (£).
    // Based on their overall rating (squared, so good players are exponentially more expensive)
    // and their age (younger players are worth more because they have more years left).
    int marketValue() const;

    // Convenience method: returns "GK", "DEF", "MID", or "FWD" as a string.
    std::string positionStr() const { return positionToString(position); }

    // Factory method that creates a brand new randomly-generated player.
    // "quality" is the base skill level (1-20). The generated player's attributes will
    // scatter around this quality level, with their primary attributes (e.g. shooting for FWD)
    // being higher and their irrelevant attributes (e.g. goalkeeping for FWD) being lower.
    static Player generate(Position pos, int quality, const std::string& playerName);
};
