// SaveLoad.cpp
// Implementation of the save/load system.
//
// SAVE FILE FORMAT:
// The file is divided into sections with headers in square brackets.
// Each section contains key=value pairs or comma-separated data.
//
// Sections:
//   [GAME]          — Season number, current week, player's team, number of teams
//   [TEAM:n]        — Team n's name, budget, and tactics
//   [PLAYER:t:p]    — Player p on team t's attributes, age, and season stats
//   [TABLE]         — League table: teamIdx,played,won,drawn,lost,goalsFor,goalsAgainst
//   [SCHEDULE]      — All fixtures: week,homeIdx,awayIdx,played[,homeGoals,awayGoals]
//
// The load function is essentially a state machine that reads the file line by line,
// switches between section states based on the headers, and fills in the League object.

#include "SaveLoad.h"
#include <fstream>
#include <sstream>

// Returns the default save file name. Just "save.dat" in the current directory.
std::string SaveLoad::defaultSavePath() {
    return "save.dat";
}

// ============================================================================
// SAVE GAME
// ============================================================================
// Writes the entire game state to a text file.
// The output is structured for easy parsing: section headers, key=value pairs,
// and CSV-formatted data for the table and schedule.
bool SaveLoad::saveGame(const League& league, int playerTeamIdx, const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    // ---- GAME HEADER ----
    // Global game state: what season, what week, which team the player controls
    out << "[GAME]\n";
    out << "season=" << league.season << "\n";
    out << "currentWeek=" << league.currentWeek << "\n";
    out << "playerTeamIdx=" << playerTeamIdx << "\n";
    out << "numTeams=" << league.teams.size() << "\n";
    out << "\n";

    // ---- TEAMS AND PLAYERS ----
    // For each team: save name, budget, tactics, then every player's full data
    for (int t = 0; t < (int)league.teams.size(); t++) {
        auto& team = league.teams[t];
        out << "[TEAM:" << t << "]\n";
        out << "name=" << team.name << "\n";
        out << "budget=" << team.budget << "\n";
        // Tactics are saved as integer enum values (e.g. Formation::F_4_4_2 = 0)
        out << "formation=" << (int)team.tactics.formation << "\n";
        out << "mentality=" << (int)team.tactics.mentality << "\n";
        out << "playStyle=" << (int)team.tactics.playStyle << "\n";
        out << "squadSize=" << team.squad.size() << "\n";
        out << "\n";

        // Save every player on this team
        for (int p = 0; p < (int)team.squad.size(); p++) {
            auto& player = team.squad[p];
            out << "[PLAYER:" << t << ":" << p << "]\n";
            out << "name=" << player.name << "\n";
            out << "age=" << player.age << "\n";
            out << "position=" << (int)player.position << "\n";  // Enum as int
            // All 8 attributes
            out << "pace=" << player.pace << "\n";
            out << "passing=" << player.passing << "\n";
            out << "shooting=" << player.shooting << "\n";
            out << "defending=" << player.defending << "\n";
            out << "dribbling=" << player.dribbling << "\n";
            out << "goalkeeping=" << player.goalkeeping << "\n";
            out << "stamina=" << player.stamina << "\n";
            out << "creativity=" << player.creativity << "\n";
            // Season stats
            out << "goals=" << player.goals << "\n";
            out << "assists=" << player.assists << "\n";
            out << "appearances=" << player.appearances << "\n";
            out << "\n";
        }
    }

    // ---- LEAGUE TABLE ----
    // One row per team, CSV format: teamIdx,played,won,drawn,lost,goalsFor,goalsAgainst
    out << "[TABLE]\n";
    for (auto& entry : league.table) {
        out << entry.teamIdx << ","
            << entry.played << ","
            << entry.won << ","
            << entry.drawn << ","
            << entry.lost << ","
            << entry.goalsFor << ","
            << entry.goalsAgainst << "\n";
    }
    out << "\n";

    // ---- SCHEDULE ----
    // First line: total number of weeks
    // Then one line per fixture: week,homeTeamIdx,awayTeamIdx,played(0/1)[,homeGoals,awayGoals]
    // The goals are only included if the match has been played.
    out << "[SCHEDULE]\n";
    out << "weeks=" << league.schedule.size() << "\n";
    for (int w = 0; w < (int)league.schedule.size(); w++) {
        for (auto& f : league.schedule[w]) {
            out << w << "," << f.homeTeamIdx << "," << f.awayTeamIdx << ","
                << (f.played ? 1 : 0);
            if (f.played) {
                out << "," << f.result.homeGoals << "," << f.result.awayGoals;
            }
            out << "\n";
        }
    }

    out.close();
    return true;
}

// ============================================================================
// LOAD GAME
// ============================================================================
// Reads a save file and reconstructs the entire League object.
//
// This is implemented as a simple state machine:
//   - Read the file line by line
//   - When we hit a section header like [GAME] or [TEAM:3], switch to that state
//   - Parse key=value pairs according to the current state
//   - For TABLE and SCHEDULE sections, parse CSV data
//
// The "getValue" lambda extracts the part after the '=' sign in a key=value line.
// For example: "budget=5000000" → returns "5000000"
bool SaveLoad::loadGame(League& league, int& playerTeamIdx, const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) return false;

    std::string line;
    league.teams.clear();
    league.table.clear();
    league.schedule.clear();

    // State machine: which section of the file are we currently reading?
    enum Section { NONE, GAME, TEAM_HDR, PLAYER, TABLE, SCHED };
    Section section = NONE;
    int curTeam = -1, curPlayer = -1;

    // Helper: given a line like "budget=5000000", returns "5000000"
    auto getValue = [](const std::string& l) -> std::string {
        auto pos = l.find('=');
        if (pos == std::string::npos) return "";
        return l.substr(pos + 1);
    };

    while (std::getline(in, line)) {
        if (line.empty()) continue;  // Skip blank lines

        // ---- DETECT SECTION HEADERS ----
        if (line == "[GAME]") { section = GAME; continue; }
        if (line.substr(0, 6) == "[TEAM:") {
            section = TEAM_HDR;
            // Extract team index from "[TEAM:3]" → 3
            curTeam = std::stoi(line.substr(6, line.find(']') - 6));
            if (curTeam >= (int)league.teams.size()) league.teams.resize(curTeam + 1);
            continue;
        }
        if (line.substr(0, 8) == "[PLAYER:") {
            section = PLAYER;
            // Extract team:player indices from "[PLAYER:2:5]" → team=2, player=5
            auto inner = line.substr(8, line.find(']') - 8);
            auto comma = inner.find(':');
            curTeam = std::stoi(inner.substr(0, comma));
            curPlayer = std::stoi(inner.substr(comma + 1));
            if (curTeam >= (int)league.teams.size()) league.teams.resize(curTeam + 1);
            auto& sq = league.teams[curTeam].squad;
            if (curPlayer >= (int)sq.size()) sq.resize(curPlayer + 1);
            continue;
        }
        if (line == "[TABLE]") { section = TABLE; continue; }
        if (line == "[SCHEDULE]") { section = SCHED; continue; }

        // ---- PARSE DATA BASED ON CURRENT SECTION ----
        switch (section) {
            case GAME:
                // Parse global game state
                if (line.find("season=") == 0) league.season = std::stoi(getValue(line));
                else if (line.find("currentWeek=") == 0) league.currentWeek = std::stoi(getValue(line));
                else if (line.find("playerTeamIdx=") == 0) playerTeamIdx = std::stoi(getValue(line));
                else if (line.find("numTeams=") == 0) league.teams.resize(std::stoi(getValue(line)));
                break;

            case TEAM_HDR: {
                // Parse team-level data (name, budget, tactics)
                auto& team = league.teams[curTeam];
                if (line.find("name=") == 0) team.name = getValue(line);
                else if (line.find("budget=") == 0) team.budget = std::stoi(getValue(line));
                // Cast integers back to enum types
                else if (line.find("formation=") == 0) team.tactics.formation = (Formation)std::stoi(getValue(line));
                else if (line.find("mentality=") == 0) team.tactics.mentality = (Mentality)std::stoi(getValue(line));
                else if (line.find("playStyle=") == 0) team.tactics.playStyle = (PlayStyle)std::stoi(getValue(line));
                else if (line.find("squadSize=") == 0) team.squad.resize(std::stoi(getValue(line)));
                break;
            }

            case PLAYER: {
                // Parse individual player data (name, age, position, attributes, stats)
                auto& p = league.teams[curTeam].squad[curPlayer];
                if (line.find("name=") == 0) p.name = getValue(line);
                else if (line.find("age=") == 0) p.age = std::stoi(getValue(line));
                else if (line.find("position=") == 0) p.position = (Position)std::stoi(getValue(line));
                else if (line.find("pace=") == 0) p.pace = std::stoi(getValue(line));
                else if (line.find("passing=") == 0) p.passing = std::stoi(getValue(line));
                else if (line.find("shooting=") == 0) p.shooting = std::stoi(getValue(line));
                else if (line.find("defending=") == 0) p.defending = std::stoi(getValue(line));
                else if (line.find("dribbling=") == 0) p.dribbling = std::stoi(getValue(line));
                else if (line.find("goalkeeping=") == 0) p.goalkeeping = std::stoi(getValue(line));
                else if (line.find("stamina=") == 0) p.stamina = std::stoi(getValue(line));
                else if (line.find("creativity=") == 0) p.creativity = std::stoi(getValue(line));
                else if (line.find("goals=") == 0) p.goals = std::stoi(getValue(line));
                else if (line.find("assists=") == 0) p.assists = std::stoi(getValue(line));
                else if (line.find("appearances=") == 0) p.appearances = std::stoi(getValue(line));
                break;
            }

            case TABLE: {
                // Parse league table: CSV line with teamIdx,played,won,drawn,lost,goalsFor,goalsAgainst
                std::istringstream ss(line);
                LeagueEntry e;
                char c;  // Used to consume the commas between values
                ss >> e.teamIdx >> c >> e.played >> c >> e.won >> c >> e.drawn >> c
                   >> e.lost >> c >> e.goalsFor >> c >> e.goalsAgainst;
                league.table.push_back(e);
                break;
            }

            case SCHED: {
                // Parse schedule: first line is "weeks=30", then fixture lines
                if (line.find("weeks=") == 0) {
                    int totalWeeks = std::stoi(getValue(line));
                    league.schedule.resize(totalWeeks);
                } else {
                    // Parse fixture: week,homeIdx,awayIdx,played[,homeGoals,awayGoals]
                    std::istringstream ss(line);
                    int week, home, away, played;
                    char c;
                    ss >> week >> c >> home >> c >> away >> c >> played;
                    Fixture f;
                    f.homeTeamIdx = home;
                    f.awayTeamIdx = away;
                    f.played = (played == 1);
                    if (f.played) {
                        // If the match was played, also read the goals
                        int hg, ag;
                        ss >> c >> hg >> c >> ag;
                        f.result.homeGoals = hg;
                        f.result.awayGoals = ag;
                        f.result.homeTeam = league.teams[home].name;
                        f.result.awayTeam = league.teams[away].name;
                    }
                    if (week < (int)league.schedule.size()) {
                        league.schedule[week].push_back(f);
                    }
                }
                break;
            }

            default: break;
        }
    }

    in.close();
    return true;
}
