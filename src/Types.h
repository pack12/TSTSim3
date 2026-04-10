// Types.h
// This file defines ALL the basic data types, enums, and structs used throughout the entire game.
// Think of it as the "dictionary" for the game — every other file references these definitions.
// If you're confused about what a "Formation" or "MatchEvent" is, this is where you look.

#pragma once  // Tells the compiler: "Only include this file once, even if multiple files #include it."
#include <string>
#include <vector>

// ============================================================================
// POSITION ENUM
// ============================================================================
// The four positions a player can play. That's it. GK = Goalkeeper, DEF = Defender,
// MID = Midfielder, FWD = Forward. "enum class" means you have to write Position::GK
// instead of just GK, which prevents name collisions with other enums.
enum class Position { GK, DEF, MID, FWD };

// ============================================================================
// FORMATION ENUM
// ============================================================================
// The formations a team can use. The numbers represent Defenders-Midfielders-Forwards.
// For example, F_4_4_2 means 4 defenders, 4 midfielders, 2 forwards (plus 1 goalkeeper = 11 total).
// COUNT is a trick — it's not a real formation, it just tells us how many formations exist.
enum class Formation {
    F_4_4_2,    // Classic English formation. Balanced.
    F_4_3_3,    // More attacking. Extra forward at the cost of a midfielder.
    F_3_5_2,    // Packed midfield. Risky at the back.
    F_4_5_1,    // Very defensive. One lone striker doing all the work up front.
    F_3_4_3,    // Ultra-attacking. Three forwards, three defenders. Pray.
    F_5_3_2,    // Park the bus with 5 defenders. Boring but effective.
    COUNT       // Not a formation! Just tells us there are 6 formations total.
};

// FormationInfo breaks down a formation into its component parts.
// "How many defenders does a 4-3-3 have?" — this struct answers that.
struct FormationInfo {
    int defenders;      // Number of defenders (not counting GK)
    int midfielders;    // Number of midfielders
    int forwards;       // Number of forwards
    std::string name;   // Human-readable name like "4-4-2" for display in the UI
};

// Given a Formation enum value, returns a FormationInfo struct with the details.
// "inline" means this function's code lives in the header file. Normally functions
// go in .cpp files, but small helper functions like this are fine in headers.
inline FormationInfo getFormationInfo(Formation f) {
    switch (f) {
        case Formation::F_4_4_2: return {4, 4, 2, "4-4-2"};
        case Formation::F_4_3_3: return {4, 3, 3, "4-3-3"};
        case Formation::F_3_5_2: return {3, 5, 2, "3-5-2"};
        case Formation::F_4_5_1: return {4, 5, 1, "4-5-1"};
        case Formation::F_3_4_3: return {3, 4, 3, "3-4-3"};
        case Formation::F_5_3_2: return {5, 3, 2, "5-3-2"};
        default: return {4, 4, 2, "4-4-2"};  // Fallback to 4-4-2 if something weird happens
    }
}

// ============================================================================
// MENTALITY ENUM
// ============================================================================
// How aggressively your team plays. Very Defensive = sit deep, don't take risks.
// Very Attacking = throw everyone forward and hope for the best.
// This directly affects the attack/defense modifiers in the match engine.
enum class Mentality {
    VERY_DEFENSIVE,     // Sit deep, barely cross halfway. Like Mourinho on a bad day.
    DEFENSIVE,          // Cautious. Protect the lead.
    BALANCED,           // Default. 50/50 approach.
    ATTACKING,          // Push forward. Take more risks.
    VERY_ATTACKING      // Everyone forward! Last 10 minutes, down 1-0 energy.
};

// Converts a Mentality enum to a nice readable string for the UI.
inline std::string mentalityToString(Mentality m) {
    switch (m) {
        case Mentality::VERY_DEFENSIVE: return "Very Defensive";
        case Mentality::DEFENSIVE: return "Defensive";
        case Mentality::BALANCED: return "Balanced";
        case Mentality::ATTACKING: return "Attacking";
        case Mentality::VERY_ATTACKING: return "Very Attacking";
    }
    return "Unknown";  // Should never happen, but compilers complain without it
}

// ============================================================================
// PLAY STYLE ENUM
// ============================================================================
// The general philosophy of how the team plays. Affects tempo, passing, pressing.
// This is separate from mentality — you can be "Defensive" mentality with "Direct" style
// (sit deep, then launch long balls when you win it back = counter-attacking football).
enum class PlayStyle {
    PARK_THE_BUS,       // 10 men behind the ball. 0 excitement. Maximum shutout potential.
    COUNTER_ATTACK,     // Defend, win the ball, sprint forward on the break.
    BALANCED,           // No particular philosophy. Jack of all trades.
    POSSESSION,         // Keep the ball. Tiki-taka. Bore them into submission.
    DIRECT,             // Long balls, skip the midfield. Route one football.
    ALL_OUT_ATTACK      // Goalkeeper is basically a striker. Pure chaos.
};

// Converts PlayStyle to a display string. Same pattern as the others.
inline std::string playStyleToString(PlayStyle ps) {
    switch (ps) {
        case PlayStyle::PARK_THE_BUS: return "Park the Bus";
        case PlayStyle::COUNTER_ATTACK: return "Counter Attack";
        case PlayStyle::BALANCED: return "Balanced";
        case PlayStyle::POSSESSION: return "Possession";
        case PlayStyle::DIRECT: return "Direct";
        case PlayStyle::ALL_OUT_ATTACK: return "All Out Attack";
    }
    return "Unknown";
}

// Converts Position enum to a short 2-3 character string. Used everywhere in the UI.
inline std::string positionToString(Position p) {
    switch (p) {
        case Position::GK: return "GK";
        case Position::DEF: return "DEF";
        case Position::MID: return "MID";
        case Position::FWD: return "FWD";
    }
    return "?";
}

// ============================================================================
// MATCH EVENT
// ============================================================================
// One thing that happened during a match. "32' — Smith shoots! Saved by the keeper!"
// The match engine generates a list of these, and the UI displays them as commentary.
struct MatchEvent {
    int minute;                 // What minute of the match did this happen (1-90+)
    std::string description;    // The commentary text, e.g. "Jones heads it in! GOAL!"
    bool isGoal = false;        // Was this event a goal? Used to update the score display.
    bool isHomeTeam = true;     // Did the home team create this event? Used for score tracking.
};

// ============================================================================
// MATCH RESULT
// ============================================================================
// The complete result of a finished match. Contains the final score AND every event
// that happened during the match (for replaying commentary or showing highlights).
struct MatchResult {
    std::string homeTeam;               // Name of the home team, e.g. "Alderwick City"
    std::string awayTeam;               // Name of the away team
    int homeGoals = 0;                  // Final number of goals scored by the home team
    int awayGoals = 0;                  // Final number of goals scored by the away team
    std::vector<MatchEvent> events;     // Every event that happened, in chronological order
};

// ============================================================================
// FIXTURE
// ============================================================================
// A scheduled match between two teams. Before the match is played, "played" is false
// and "result" is empty. After simulation, "played" becomes true and "result" is filled in.
// Think of this as one row in the fixture list: "Week 5: Alderwick City vs Glendale Rovers"
struct Fixture {
    int homeTeamIdx;        // Index into the League's teams[] vector for the home team
    int awayTeamIdx;        // Index into the League's teams[] vector for the away team
    bool played = false;    // Has this match been simulated yet?
    MatchResult result;     // The result (only meaningful if played == true)
};

// ============================================================================
// LEAGUE TABLE ENTRY
// ============================================================================
// One row in the league table. Tracks wins, losses, draws, goals for/against for a single team.
// The league table is just a vector of these, sorted by points (then goal difference, then goals scored).
struct LeagueEntry {
    int teamIdx;            // Which team this row is for (index into League::teams[])
    int played = 0;         // Total matches played so far
    int won = 0;            // Total matches won
    int drawn = 0;          // Total matches drawn
    int lost = 0;           // Total matches lost
    int goalsFor = 0;       // Total goals this team has scored across all matches
    int goalsAgainst = 0;   // Total goals conceded

    // Standard football points: 3 for a win, 1 for a draw, 0 for a loss.
    int points() const { return won * 3 + drawn; }

    // Goal difference = goals scored minus goals conceded. Used as a tiebreaker in the table.
    // Positive = you score more than you concede. Negative = you're having a bad time.
    int goalDifference() const { return goalsFor - goalsAgainst; }
};
