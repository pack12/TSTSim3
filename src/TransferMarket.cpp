// TransferMarket.cpp
// Implementation of the transfer market. Handles generating listings, buying, and selling.

#include "TransferMarket.h"
#include <random>
#include <algorithm>
#include <numeric>

// Shared random number generator for transfer market operations
static std::mt19937& rng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

// ============================================================================
// GENERATE LISTINGS
// ============================================================================
// Creates the initial set of transfer listings from all AI teams.
//
// For each AI team (excluding the player's team):
//   1. Decide how many players to list (randomly 1 to 3)
//   2. Shuffle the squad indices to pick random players
//   3. For each listed player, calculate an asking price:
//      - Start with the player's calculated market value
//      - Multiply by a random factor between 0.8x and 1.3x (some deals, some rip-offs)
//      - Round down to the nearest £100,000 for cleanliness
//      - Minimum price: £100,000 (nobody is free)
//   4. Sort all listings by overall rating so the best players appear first
void TransferMarket::generateListings(League& league, int playerTeamIdx) {
    listings.clear();

    for (int t = 0; t < (int)league.teams.size(); t++) {
        if (t == playerTeamIdx) continue;  // Skip the player's own team

        auto& team = league.teams[t];
        int numListings = std::uniform_int_distribution<>(1, 3)(rng());  // List 1-3 players

        // Shuffle squad indices so we pick random players, not always the first few
        std::vector<int> indices(team.squad.size());
        std::iota(indices.begin(), indices.end(), 0);  // Fill with 0, 1, 2, ...
        std::shuffle(indices.begin(), indices.end(), rng());

        for (int i = 0; i < numListings && i < (int)indices.size(); i++) {
            int pidx = indices[i];
            TransferListing listing;
            listing.teamIdx = t;
            listing.playerIdx = pidx;
            // Calculate asking price: market value * random multiplier (0.8-1.3)
            int base = team.squad[pidx].marketValue();
            float mult = 0.8f + std::uniform_real_distribution<float>(0.0f, 0.5f)(rng());
            listing.askingPrice = (int)(base * mult);
            listing.askingPrice = (listing.askingPrice / 100000) * 100000;  // Round to nearest 100k
            if (listing.askingPrice < 100000) listing.askingPrice = 100000; // Minimum £100k
            listings.push_back(listing);
        }
    }

    // Sort listings by player overall rating, highest first (best players at the top)
    std::sort(listings.begin(), listings.end(), [&](auto& a, auto& b) {
        return league.teams[a.teamIdx].squad[a.playerIdx].overall() >
               league.teams[b.teamIdx].squad[b.playerIdx].overall();
    });
}

// ============================================================================
// BUY PLAYER
// ============================================================================
// Attempts to purchase a listed player.
//
// Checks:
//   1. Valid listing index? (bounds check)
//   2. Does the buyer have enough money?
//   3. Does the seller have at least 15 players? (can't go below 14 after selling)
//
// If all checks pass:
//   1. Copy the player to the buyer's squad
//   2. Deduct money from buyer, add to seller
//   3. Remove the player from the seller's squad
//   4. Remove this listing from the market
//   5. IMPORTANT: Adjust remaining listing indices for the same team, because erasing
//      a player from the seller's squad shifts all subsequent indices down by 1.
//      Without this fix, later purchases from the same team would reference wrong players.
bool TransferMarket::buyPlayer(League& league, int playerTeamIdx, int listingIdx) {
    if (listingIdx < 0 || listingIdx >= (int)listings.size()) return false;

    auto listing = listings[listingIdx];  // Copy, not reference (we're about to erase it)
    auto& buyerTeam = league.teams[playerTeamIdx];
    auto& sellerTeam = league.teams[listing.teamIdx];

    // Budget check
    if (buyerTeam.budget < listing.askingPrice) return false;
    // Minimum squad size check for the seller
    if ((int)sellerTeam.squad.size() <= 14) return false;

    // Execute the transfer
    Player player = sellerTeam.squad[listing.playerIdx];  // Copy the player
    buyerTeam.budget -= listing.askingPrice;               // Buyer pays
    sellerTeam.budget += listing.askingPrice;               // Seller receives
    buyerTeam.squad.push_back(player);                     // Player joins buyer
    sellerTeam.squad.erase(sellerTeam.squad.begin() + listing.playerIdx);  // Player leaves seller

    // Fix up remaining listings: when we erase a player from the seller's squad,
    // every player AFTER that index shifts down by 1. We need to update the
    // playerIdx in any other listings for the same team.
    int soldTeam = listing.teamIdx;
    int soldIdx = listing.playerIdx;
    listings.erase(listings.begin() + listingIdx);  // Remove the completed listing
    for (auto& l : listings) {
        if (l.teamIdx == soldTeam && l.playerIdx > soldIdx) {
            l.playerIdx--;  // Shift down because the squad vector got shorter
        }
    }

    return true;
}

// ============================================================================
// SELL PLAYER
// ============================================================================
// Sells one of the player's own squad members.
// The player doesn't go to another team — they just disappear (simplification).
//
// Sale price: 70-90% of market value (you always lose money selling, just like real football
// agents take their cut and there's depreciation).
// Rounded to nearest £100,000, minimum £50,000.
bool TransferMarket::sellPlayer(League& league, int playerTeamIdx, int squadIdx) {
    auto& team = league.teams[playerTeamIdx];
    if (squadIdx < 0 || squadIdx >= (int)team.squad.size()) return false;
    if ((int)team.squad.size() <= 14) return false;  // Can't go below 14 players

    // Calculate sale price: 70-90% of market value
    int value = team.squad[squadIdx].marketValue();
    float mult = 0.7f + std::uniform_real_distribution<float>(0.0f, 0.2f)(rng());
    int salePrice = (int)(value * mult);
    salePrice = (salePrice / 100000) * 100000;  // Round to nearest 100k
    if (salePrice < 50000) salePrice = 50000;   // Minimum £50k

    team.budget += salePrice;  // Add the money to the team's budget
    team.squad.erase(team.squad.begin() + squadIdx);  // Remove the player from the squad

    return true;
}

// Just regenerates all listings from scratch. Called when the player clicks "Refresh".
void TransferMarket::refreshListings(League& league, int playerTeamIdx) {
    generateListings(league, playerTeamIdx);
}
