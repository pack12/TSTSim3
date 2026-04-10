// League.cpp
// Implementation of the League class. Contains the schedule generation algorithm,
// table management, and league initialization.

#include "League.h"
#include "DataGen.h"
#include <algorithm>
#include <numeric>
#include <random>

// Separate RNG for league operations (schedule shuffling).
static std::mt19937& leagueRng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

// ============================================================================
// INITIALIZE
// ============================================================================
// Sets up the entire league from scratch. This is called when starting a new game.
// Steps:
//   1. Generate all 16 teams from the templates in DataGen.h (each with a name + quality)
//   2. Create a blank league table with one entry per team (all stats at zero)
//   3. Generate the full season schedule (30 weeks of fixtures)
//   4. Reset the week counter to 0
void League::initialize() {
    teams.clear();
    for (auto& tmpl : DataGen::TEAM_TEMPLATES) {
        teams.push_back(Team::generate(tmpl.name, tmpl.quality));
    }

    // Create a blank league table — one entry per team, all zeros
    table.clear();
    for (int i = 0; i < (int)teams.size(); i++) {
        LeagueEntry entry;
        entry.teamIdx = i;
        table.push_back(entry);
    }

    generateSchedule();
    currentWeek = 0;
}

// ============================================================================
// GENERATE SCHEDULE
// ============================================================================
// Creates the full season schedule using the "circle method" for round-robin tournaments.
//
// HOW IT WORKS:
// In a league with N teams, each team must play every other team twice (home and away).
// That's (N-1) * 2 weeks total. For 16 teams: 15 * 2 = 30 weeks.
//
// The "circle method" works like this:
//   - Team 0 is fixed in place. Teams 1 through N-1 rotate around a circle.
//   - Each "round" of the circle produces one week of fixtures (N/2 matches per week).
//   - We first generate the "first half" of the season (15 weeks, each team plays
//     every other team once). Then we mirror it for the "second half" by swapping
//     home and away for every fixture.
//
// The shuffle at the start randomizes which teams end up in which slots,
// so the schedule is different each time. We shuffle indices 1..N-1 but keep
// index 0 fixed (the "pivot" of the circle method).
//
// This is the same algorithm used by real football leagues to generate their fixtures.
void League::generateSchedule() {
    int n = (int)teams.size();
    schedule.clear();

    // Create team indices [0, 1, 2, ..., n-1] and shuffle all except index 0
    std::vector<int> ids(n);
    std::iota(ids.begin(), ids.end(), 0);                 // Fill with 0, 1, 2, ..., n-1
    std::shuffle(ids.begin() + 1, ids.end(), leagueRng()); // Shuffle everyone except the first

    // FIRST HALF: Generate n-1 rounds (each team plays every other team once)
    for (int round = 0; round < n - 1; round++) {
        std::vector<Fixture> week;
        for (int match = 0; match < n / 2; match++) {
            int idx1, idx2;
            if (match == 0) {
                // First match always involves the fixed team (index 0)
                // paired with the team at position (round % (n-1)) + 1
                idx1 = 0;
                idx2 = (round % (n - 1)) + 1;
            } else {
                // Remaining matches: pair up teams from opposite ends of the rotation
                idx1 = ((round + match) % (n - 1)) + 1;
                idx2 = ((round + n - 1 - match) % (n - 1)) + 1;
            }

            Fixture f;
            f.homeTeamIdx = ids[idx1];  // Map the rotation index back to a real team
            f.awayTeamIdx = ids[idx2];
            week.push_back(f);
        }
        schedule.push_back(week);
    }

    // SECOND HALF: Mirror the first half with home/away swapped.
    // This means every team plays every opponent twice: once at home, once away.
    int firstHalfWeeks = (int)schedule.size();
    for (int round = 0; round < firstHalfWeeks; round++) {
        std::vector<Fixture> week;
        for (auto& f : schedule[round]) {
            Fixture rev;
            rev.homeTeamIdx = f.awayTeamIdx;  // Swap home and away
            rev.awayTeamIdx = f.homeTeamIdx;
            week.push_back(rev);
        }
        schedule.push_back(week);
    }
}

// ============================================================================
// UPDATE TABLE
// ============================================================================
// After a match is played, this method updates both teams' entries in the league table.
// It adds goals for/against, increments the appropriate W/D/L counter, and re-sorts.
//
// Standard football points:
//   Win  = 3 points
//   Draw = 1 point
//   Loss = 0 points
void League::updateTable(const Fixture& fixture) {
    // Lambda to find the table entry for a given team index
    auto findEntry = [&](int teamIdx) -> LeagueEntry& {
        for (auto& e : table) {
            if (e.teamIdx == teamIdx) return e;
        }
        return table[0];  // Fallback (should never happen)
    };

    auto& home = findEntry(fixture.homeTeamIdx);
    auto& away = findEntry(fixture.awayTeamIdx);

    int hg = fixture.result.homeGoals;
    int ag = fixture.result.awayGoals;

    // Update matches played and goals for both teams
    home.played++;
    away.played++;
    home.goalsFor += hg;
    home.goalsAgainst += ag;
    away.goalsFor += ag;
    away.goalsAgainst += hg;

    // Determine the result and update W/D/L accordingly
    if (hg > ag) {
        home.won++;       // Home team wins
        away.lost++;
    } else if (hg < ag) {
        away.won++;       // Away team wins
        home.lost++;
    } else {
        home.drawn++;     // Draw — both teams get 1 point
        away.drawn++;
    }

    sortTable();  // Re-sort the table after every update
}

// ============================================================================
// SORT TABLE
// ============================================================================
// Sorts the league table in standard football order:
//   1. Points (descending) — most points = highest position
//   2. Goal difference (descending) — tiebreaker #1
//   3. Goals scored (descending) — tiebreaker #2 (rewards attacking football)
void League::sortTable() {
    std::sort(table.begin(), table.end(), [](const LeagueEntry& a, const LeagueEntry& b) {
        if (a.points() != b.points()) return a.points() > b.points();
        if (a.goalDifference() != b.goalDifference()) return a.goalDifference() > b.goalDifference();
        return a.goalsFor > b.goalsFor;
    });
}

// Returns the fixtures for the current week. Called by MainWindow when simulating matches.
std::vector<Fixture>& League::currentFixtures() {
    return schedule[currentWeek];
}

// Returns true if every week in the season has been played.
// The currentWeek counter is incremented after each round of matches.
bool League::seasonComplete() const {
    return currentWeek >= (int)schedule.size();
}
