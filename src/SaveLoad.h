// SaveLoad.h
// Save and load system for the game. Uses a simple text-based file format.
//
// The save file is human-readable (you could open it in a text editor) and uses
// section headers like [GAME], [TEAM:0], [PLAYER:0:3], [TABLE], [SCHEDULE].
// Each piece of data is stored as a key=value pair on its own line.
//
// This is intentionally simple rather than using JSON or binary serialization,
// because it's easy to debug and understand, and the data is small enough that
// performance doesn't matter.

#pragma once
#include "League.h"
#include <string>

namespace SaveLoad {
    // Saves the entire game state to a file.
    // Writes: season number, current week, player's team index, all 16 teams with
    // full squad details (every player attribute and stat), the league table, and
    // the complete schedule (including results of played matches).
    // Returns true on success, false if the file couldn't be opened.
    bool saveGame(const League& league, int playerTeamIdx, const std::string& filename);

    // Loads a saved game from a file.
    // Reconstructs the entire League object from the save file data.
    // Also restores the player's team index (which team you're managing).
    // Returns true on success, false if the file doesn't exist or can't be read.
    bool loadGame(League& league, int& playerTeamIdx, const std::string& filename);

    // Returns the default save file path ("save.dat" in the current working directory).
    std::string defaultSavePath();
}
