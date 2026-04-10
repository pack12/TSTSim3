// TransferMarket.h
// Defines the transfer market system — where you buy and sell players.
//
// The transfer market works like this:
//   - Each AI team lists 1-3 random players for sale (your team never auto-lists players)
//   - Asking prices are 80-130% of the player's calculated market value
//   - You can buy any listed player if you have the budget
//   - You can sell any of your own players for 70-90% of market value (discount because you're selling)
//   - You can refresh the listings to see new players available
//   - Teams can't sell below a minimum squad size of 14 (so they don't sell everyone)

#pragma once
#include "League.h"
#include <vector>

// Represents one player listed on the transfer market.
// It's basically: "Team X is selling Player Y for Z pounds."
struct TransferListing {
    int teamIdx;        // Which team is selling this player (index into League::teams[])
    int playerIdx;      // Which player in that team's squad (index into Team::squad[])
    int askingPrice;    // How much they want for the player, in pounds
};

class TransferMarket {
public:
    std::vector<TransferListing> listings;  // All currently available transfer listings

    // Generates fresh transfer listings from all AI teams.
    // Each AI team randomly lists 1-3 players at 80-130% of market value.
    // The player's own team is excluded (you can't buy your own players).
    // Listings are sorted by overall rating (best players first).
    void generateListings(League& league, int playerTeamIdx);

    // Attempts to buy a player from the market.
    // Checks: do you have enough money? Does the selling team have enough players (>14)?
    // If successful: player moves to your squad, money changes hands, listing is removed,
    // and remaining listing indices are adjusted (because erasing a player shifts indices).
    // Returns true on success, false on failure.
    bool buyPlayer(League& league, int playerTeamIdx, int listingIdx);

    // Sells one of your own players for 70-90% of their market value.
    // The player just disappears (they don't join another team — simplification).
    // Minimum squad size of 14 is enforced.
    // Returns true on success, false on failure.
    bool sellPlayer(League& league, int playerTeamIdx, int squadIdx);

    // Regenerates the listings from scratch. Just calls generateListings() again.
    // Used when the player wants to see a different set of available players.
    void refreshListings(League& league, int playerTeamIdx);
};
