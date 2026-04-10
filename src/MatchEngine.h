// MatchEngine.h
// Defines the MatchEngine class — the heart of the game.
// This is the thing that actually simulates a football match, minute by minute.
//
// The match engine doesn't know about Qt or the UI. It takes two Team objects,
// runs a probabilistic simulation, and returns a MatchResult containing the final score
// and a list of MatchEvents (the commentary — "Smith shoots! GOAL!" etc.).
//
// It can optionally call a callback function for each event as it happens,
// which is how the MatchWidget shows live commentary during the user's match.

#pragma once
#include "Team.h"
#include "Types.h"
#include <vector>
#include <string>
#include <functional>

class MatchEngine {
public:
    // Callback type for live commentary. The match engine calls this function every time
    // something happens in the match, passing the minute, commentary text, and whether it's a goal.
    // This is how the UI shows events one at a time during the user's match.
    using CommentaryCallback = std::function<void(int minute, const std::string& text, bool isGoal)>;

    // Simulates an entire match between home and away teams.
    // Returns a MatchResult with the final score and all events.
    //
    // Parameters:
    //   home:        The home team (gets a slight advantage in possession)
    //   away:        The away team
    //   interactive: If true, the callback is used for live commentary (not currently used — events are
    //                stored and replayed by MatchWidget instead)
    //   callback:    Optional function called for each match event as it happens
    MatchResult simulate(Team& home, Team& away, bool interactive = false,
                         CommentaryCallback callback = nullptr);

private:
    // Internal state tracked during a match simulation.
    // Keeps running totals of goals and a list of all events so far.
    struct MatchState {
        int homeGoals = 0;
        int awayGoals = 0;
        std::vector<MatchEvent> events;
    };

    // Determines which team has possession for this minute of the match.
    // Based on midfield strength of both teams, modified by tactics.
    // Home team gets a slight 8% advantage (home crowd effect).
    // Returns true if the home team wins possession, false if away.
    bool determinePossession(Team& home, Team& away);

    // Simulates one attacking move by the attacking team.
    // Goes through phases: build-up → progression → chance creation → shot → resolution.
    // At each phase, the attack can fail (ball lost, pass intercepted, shot saved, etc.).
    // Returns commentary text describing what happened. If a goal is scored,
    // the text will contain "GOAL!" and the scorer's stats are updated.
    std::string simulateAttack(Team& attacker, Team& defender, int minute,
                               MatchState& state, bool isHome);

    // Picks a random player from the team's starting XI at the given position.
    // Used to generate realistic commentary (e.g. "Smith plays it to Jones").
    // If no players exist at that position, falls back to any non-GK outfield player.
    std::string pickPlayer(Team& team, Position pos);
};
