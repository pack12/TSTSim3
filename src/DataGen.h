// DataGen.h
// This is the "data file" for the entire game. It contains:
//   1. A pool of 80 first names and 80 last names used to randomly generate player names.
//   2. The 16 team templates that define every team in the league.
//   3. A function to smash a random first + last name together.
//
// Everything is "inline" which means the data lives right here in the header file.
// Normally you'd put data in a .cpp file, but for a small game like this it's fine.
// The compiler just inlines it wherever it's needed.

#pragma once
#include <string>
#include <vector>
#include <random>

namespace DataGen {

// ============================================================================
// FIRST NAMES (80 total)
// ============================================================================
// A mix of classic English names (James, John, Robert) and more modern ones
// (Callum, Finn, Rhys). When generating a player, we pick one of these at random.
inline const std::vector<std::string> FIRST_NAMES = {
    "James", "John", "Robert", "Michael", "William", "David", "Richard", "Thomas",
    "Daniel", "Matthew", "Anthony", "Mark", "Steven", "Paul", "Andrew", "Joshua",
    "Kenneth", "Kevin", "Brian", "George", "Timothy", "Ronald", "Edward", "Jason",
    "Jeffrey", "Ryan", "Jacob", "Gary", "Nicholas", "Eric", "Jonathan", "Stephen",
    "Larry", "Justin", "Scott", "Brandon", "Benjamin", "Samuel", "Raymond", "Gregory",
    "Frank", "Alexander", "Patrick", "Jack", "Dennis", "Jerry", "Tyler", "Aaron",
    "Jose", "Adam", "Nathan", "Henry", "Douglas", "Peter", "Zachary", "Kyle",
    "Marcus", "Oliver", "Liam", "Ethan", "Mason", "Logan", "Lucas", "Harry",
    "Charlie", "Oscar", "Archie", "Leo", "Alfie", "Freddie", "Callum", "Connor",
    "Declan", "Finn", "Rhys", "Kieran", "Owen", "Reece", "Cameron", "Jamie"
};

// ============================================================================
// LAST NAMES (80 total)
// ============================================================================
// Classic surnames. With 80 first x 80 last, there are 6,400 possible combinations,
// which is plenty for a 16-team league with ~18 players each (288 players total).
// Duplicates are technically possible but rare enough not to matter.
inline const std::vector<std::string> LAST_NAMES = {
    "Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", "Miller", "Davis",
    "Rodriguez", "Martinez", "Wilson", "Anderson", "Taylor", "Thomas", "Moore", "Jackson",
    "Martin", "Lee", "Thompson", "White", "Harris", "Clark", "Lewis", "Robinson",
    "Walker", "Young", "Allen", "King", "Wright", "Scott", "Adams", "Baker",
    "Carter", "Mitchell", "Roberts", "Turner", "Phillips", "Campbell", "Parker", "Evans",
    "Edwards", "Collins", "Stewart", "Morris", "Murphy", "Cook", "Rogers", "Morgan",
    "Cooper", "Richardson", "Cox", "Howard", "Ward", "Brooks", "Kelly", "Bennett",
    "Wood", "Barnes", "Ross", "Henderson", "Coleman", "Jenkins", "Perry", "Powell",
    "Russell", "Sullivan", "Bell", "Reed", "Bailey", "Hunt", "Palmer", "Grant",
    "Dixon", "Burns", "Shaw", "Walsh", "Gibson", "Fletcher", "Webb", "Simpson"
};

// ============================================================================
// TEAM TEMPLATES
// ============================================================================
// Each template defines a team name and a "quality" rating (1-20).
// Quality determines how good the team's players are when generated.
// The best team (Alderwick City) has quality 16, and the worst (Ravensbrook Town)
// has quality 8. This creates a natural hierarchy where some teams are favorites
// and others are underdogs — just like a real league.
//
// The quality spread (8-16) means:
//   - Top teams: Players with attributes mostly 14-20 (elite squad)
//   - Mid teams: Players with attributes mostly 10-14 (decent but not scary)
//   - Bottom teams: Players with attributes mostly 6-10 (relegation fodder)
struct TeamTemplate {
    std::string name;   // The team's name, e.g. "Alderwick City"
    int quality;        // Base quality level for generated players (1-20 scale)
};

inline const std::vector<TeamTemplate> TEAM_TEMPLATES = {
    {"Alderwick City",      16},    // The Manchester City of this league
    {"Bridgeford United",   15},    // Strong contender
    {"Castleton Town",      14},    // Upper mid-table
    {"Dunmore Athletic",    14},    // Upper mid-table
    {"Easthill Rangers",    13},    // Solid mid-table
    {"Fairhaven FC",        13},    // Solid mid-table
    {"Glendale Rovers",     12},    // Average
    {"Hartfield City",      12},    // Average
    {"Ironbridge Town",     11},    // Below average
    {"Kingswood United",    11},    // Below average
    {"Langley Athletic",    10},    // Lower mid-table
    {"Millhaven FC",        10},    // Lower mid-table
    {"Northcroft Rangers",   9},    // Struggling
    {"Oakdale Rovers",       9},    // Struggling
    {"Pinewood City",        8},    // Relegation zone
    {"Ravensbrook Town",     8},    // Relegation zone (the Burnley of this league)
};

// ============================================================================
// NAME GENERATOR
// ============================================================================
// Picks a random first name and a random last name, slaps them together.
// The RNG (random number generator) is passed in from outside so that the caller
// controls the random seed — this is important for reproducible game generation.
//
// Example output: "James Smith", "Finn Henderson", "Oscar Fletcher"
inline std::string generatePlayerName(std::mt19937& rng) {
    // Pick a random index from 0 to (size - 1) for each name pool
    std::string first = FIRST_NAMES[std::uniform_int_distribution<>(0, (int)FIRST_NAMES.size() - 1)(rng)];
    std::string last = LAST_NAMES[std::uniform_int_distribution<>(0, (int)LAST_NAMES.size() - 1)(rng)];
    return first + " " + last;
}

}  // namespace DataGen
