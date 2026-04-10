// League.h
// Defines the League class — the container for the entire football league.
// A league has 16 teams, a full season schedule (home and away fixtures for every pair),
// a league table tracking standings, and a week counter tracking progress through the season.
//
// The league is the "god object" of the game. Almost every screen needs access to it
// because it holds the teams, the table, and the schedule. The MainWindow owns the league
// and passes references to it whenever a widget needs data.

#pragma once
#include "Team.h"
#include "Types.h"
#include <vector>

class League {
public:
    std::vector<Team> teams;                        // All 16 teams in the league
    std::vector<std::vector<Fixture>> schedule;     // 2D array: schedule[week][matchIndex] = Fixture
    std::vector<LeagueEntry> table;                 // The league table (one entry per team, sorted by points)
    int currentWeek = 0;                            // Which week we're currently on (0-indexed)
    int season = 1;                                 // Current season number (starts at 1, increments each year)

    // Sets up the entire league from scratch:
    // Creates all 16 teams from templates, builds a blank league table, and generates the schedule.
    void initialize();

    // Creates the full season schedule using the "circle method" round-robin algorithm.
    // Produces (N-1)*2 weeks of fixtures (N=16 teams → 30 weeks), where every team plays
    // every other team twice: once at home and once away.
    void generateSchedule();

    // Updates the league table after a match has been played.
    // Adds goals, increments wins/draws/losses, and re-sorts the table.
    void updateTable(const Fixture& fixture);

    // Sorts the league table by: points (descending) → goal difference → goals scored.
    // This is the standard football tiebreaker order.
    void sortTable();

    // Returns a reference to the current week's fixtures.
    // This is what gets simulated when the player clicks "Play Next Match".
    std::vector<Fixture>& currentFixtures();

    // Returns true if all weeks in the season have been played.
    bool seasonComplete() const;

    // How many weeks are in the season (should be 30 for a 16-team league).
    int totalWeeks() const { return (int)schedule.size(); }
};
