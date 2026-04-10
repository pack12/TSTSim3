// MatchEngine.cpp
// The beating heart of the game — the match simulation engine.
// This file contains ALL the logic for simulating a football match.
//
// OVERALL FLOW:
//   1. Loop through each minute of the match (90 + random injury time of 1-5 minutes)
//   2. Each minute: roll dice to see if anything interesting happens (~35% base chance)
//   3. If something happens: determine which team has the ball (possession roll)
//   4. The team with the ball tries to attack through several phases:
//      a. Build-up: Can the midfield get the ball forward? (35% chance of failure)
//      b. Progression: Can the attack beat the defense? (depends on attack vs defense strength)
//      c. Chance creation: What type of chance is it? (long shot, through ball, cross, or corner)
//      d. Resolution: Does it result in a goal, save, miss, or block?
//   5. Generate commentary text for each event and track goals/assists in player stats

#include "MatchEngine.h"
#include <random>
#include <sstream>

// Shared random number generator for match simulation
static std::mt19937& rng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

// Returns a random float between 0.0 and 1.0. Used for probability checks throughout.
static float randFloat() {
    return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng());
}

// Returns a random integer between lo and hi (inclusive).
static int randInt(int lo, int hi) {
    return std::uniform_int_distribution<>(lo, hi)(rng());
}

// ============================================================================
// PICK PLAYER
// ============================================================================
// Selects a random player name from the starting XI at the given position.
// Used to make commentary realistic — instead of "a player shoots", we get "Smith shoots".
//
// If no starters play the requested position (e.g. asking for a FWD when using a
// weird formation), falls back to picking any non-goalkeeper outfield player.
// If somehow nobody is available (shouldn't happen), returns "Unknown".
std::string MatchEngine::pickPlayer(Team& team, Position pos) {
    std::vector<int> xi = team.getStartingEleven();
    std::vector<int> candidates;
    // First try: find starters at the requested position
    for (int idx : xi) {
        if (team.squad[idx].position == pos) candidates.push_back(idx);
    }
    // Fallback: if no one plays that position, use any non-GK starter
    if (candidates.empty()) {
        for (int idx : xi) {
            if (team.squad[idx].position != Position::GK) candidates.push_back(idx);
        }
    }
    if (candidates.empty()) return "Unknown";
    // Pick one at random from the candidates
    int chosen = candidates[randInt(0, (int)candidates.size() - 1)];
    return team.squad[chosen].name;
}

// ============================================================================
// DETERMINE POSSESSION
// ============================================================================
// Decides which team gets the ball this minute. Based on midfield strength of both teams,
// modified by each team's tactical midfield modifier.
//
// The home team gets a 1.08x multiplier — a slight home advantage that simulates
// the effect of playing in front of your own crowd (studies show home teams win ~46%
// of matches in real football, so a small boost is realistic).
//
// The probability is calculated as: homeMid / (homeMid + awayMid)
// So if home midfield is twice as strong as away, home gets the ball ~67% of the time.
bool MatchEngine::determinePossession(Team& home, Team& away) {
    float homeMid = home.midfieldStrength() * home.tactics.midfieldModifier();
    float awayMid = away.midfieldStrength() * away.tactics.midfieldModifier();
    homeMid *= 1.08f;  // Home advantage
    float homeChance = homeMid / (homeMid + awayMid);
    return randFloat() < homeChance;
}

// ============================================================================
// SIMULATE (MAIN MATCH LOOP)
// ============================================================================
// Runs the entire 90-minute match (plus injury time) and returns the result.
//
// For each minute:
//   1. Check if we've entered injury time (minute 91) — announce how much was added
//   2. Roll for an "event chance" — base 35%, modified by both teams' tempo
//      (higher tempo = more events = more exciting match)
//   3. In the first 10 and last 10 minutes, add +10% event chance (matches are
//      more eventful at kickoff and when teams are desperate near the end)
//   4. If an event happens: determine possession, simulate the attack, record the event
//   5. If the event contains "GOAL!", update the score
//
// After the 90+ minutes: increment appearance stats for all starting XI players.
MatchResult MatchEngine::simulate(Team& home, Team& away, bool interactive,
                                   CommentaryCallback callback) {
    MatchState state;
    int injuryTime = randInt(1, 5);        // Random injury time between 1 and 5 minutes
    int totalMinutes = 90 + injuryTime;

    for (int minute = 1; minute <= totalMinutes; minute++) {
        // Announce injury time when we hit minute 91
        if (minute == 91 && interactive && callback) {
            callback(90, "The fourth official signals " + std::to_string(injuryTime) +
                     " minutes of added time.", false);
        }

        // Calculate the chance of something happening this minute.
        // Base rate: 35%. Modified by the average tempo of both teams.
        // First 10 mins and last 10 mins get a +10% bonus (more action at start/end).
        float eventChance = 0.35f;
        eventChance *= (home.tactics.tempoModifier() + away.tactics.tempoModifier()) / 2.0f;
        if (minute <= 10 || minute >= 80) eventChance += 0.1f;

        // Roll the dice — if nothing happens this minute, skip to the next
        if (randFloat() > eventChance) continue;

        // Something is happening! Determine who has the ball.
        bool homeAttacking = determinePossession(home, away);
        Team& attacker = homeAttacking ? home : away;
        Team& defender = homeAttacking ? away : home;

        // Simulate the attacking move and get the commentary text
        std::string commentary = simulateAttack(attacker, defender, minute, state, homeAttacking);

        if (!commentary.empty()) {
            // Build the match event
            MatchEvent event;
            event.minute = minute;
            event.description = commentary;
            event.isHomeTeam = homeAttacking;

            // Check if a goal was scored by looking for "GOAL!" in the commentary.
            // Yes, this is string matching — it's simple and it works.
            if (commentary.find("GOAL!") != std::string::npos) {
                event.isGoal = true;
                if (homeAttacking) state.homeGoals++;
                else state.awayGoals++;
            }

            state.events.push_back(event);

            // If running interactively, fire the callback so the UI can show it
            if (interactive && callback) {
                callback(minute, commentary, event.isGoal);
            }
        }
    }

    // Build the final result
    MatchResult result;
    result.homeTeam = home.name;
    result.awayTeam = away.name;
    result.homeGoals = state.homeGoals;
    result.awayGoals = state.awayGoals;
    result.events = state.events;

    // Update appearance stats for all players who started
    auto updateAppearances = [](Team& team) {
        auto xi = team.getStartingEleven();
        for (int idx : xi) team.squad[idx].appearances++;
    };
    updateAppearances(home);
    updateAppearances(away);

    return result;
}

// ============================================================================
// SIMULATE ATTACK
// ============================================================================
// This is the big one. Simulates a single attacking move through several phases.
// Each phase can end the attack (ball lost, blocked, saved) or advance to the next.
//
// PHASE 1: BUILD-UP (35% failure rate)
//   The midfielder tries to get the ball forward. Can lose it, get dispossessed,
//   or have a pass intercepted. If this fails, the attack is over — boring commentary.
//
// PHASE 2: PROGRESSION (defense-dependent)
//   The attack tries to break through the defense. The failure rate depends on
//   the ratio of defensive strength to attacking strength. Stronger defenses
//   cut out more attacks. If this fails, we get "tackled" or "pass intercepted" commentary.
//
// PHASE 3: CHANCE CREATION (random type selection)
//   If the attack gets past the defense, we determine what type of chance it is:
//     - Long shot (25%): A speculative effort from ~25 yards
//     - Through ball (30%): Midfielder plays the forward through on goal
//     - Cross (25%): Wide player swings a ball into the box
//     - Corner (20%): Set piece from a corner kick
//
// PHASE 4: RESOLUTION (each chance type has its own probabilities)
//   The chance either results in a goal, a save, a miss, or a block.
//   Goal probabilities depend on attacking strength and goalkeeper strength.
//   When a goal is scored, we update the scorer's and assister's stats.
std::string MatchEngine::simulateAttack(Team& attacker, Team& defender, int /*minute*/,
                                         MatchState& state, bool isHome) {
    std::ostringstream oss;  // String builder for commentary

    // Get the relevant strength values, modified by tactics
    float atkStr = attacker.attackStrength() * attacker.tactics.attackModifier();
    float defStr = defender.defenseStrength() * defender.tactics.defenseModifier();
    float gkStr = defender.goalkeeperStrength();

    // Pick named players for the commentary
    std::string midfielder = pickPlayer(attacker, Position::MID);
    std::string forward = pickPlayer(attacker, Position::FWD);
    std::string defPlayer = pickPlayer(defender, Position::DEF);

    // ---- PHASE 1: BUILD-UP ----
    // 35% chance the attack fizzles out in midfield before anything happens
    float buildUpRoll = randFloat();

    if (buildUpRoll < 0.35f) {
        // Attack fails in the build-up phase. Pick a random type of failure.
        int r = randInt(0, 2);
        if (r == 0) oss << midfielder << " loses the ball in midfield.";
        else if (r == 1) oss << midfielder << " is dispossessed by " << pickPlayer(defender, Position::MID) << ".";
        else oss << midfielder << "'s pass is intercepted by " << defPlayer << ".";
        return oss.str();
    }

    // ---- PHASE 2: PROGRESSION ----
    // The attack tries to get past the defense. Failure rate is proportional to
    // the defense strength relative to attack strength.
    // Formula: failThreshold = (defStr / (atkStr + defStr)) * 0.6
    // So if defense is twice as strong as attack, failThreshold ≈ 0.4 (40% block rate)
    // If they're equal, failThreshold ≈ 0.3 (30% block rate)
    float progressRoll = randFloat();
    float progressThreshold = defStr / (atkStr + defStr);

    if (progressRoll < progressThreshold * 0.6f) {
        // Defense wins — the attack is stopped
        int r = randInt(0, 3);
        if (r == 0) oss << midfielder << " plays a through ball to " << forward << " but " << defPlayer << " reads it well.";
        else if (r == 1) oss << midfielder << " finds " << forward << " who is tackled by " << defPlayer << ".";
        else if (r == 2) oss << forward << " tries to get past " << defPlayer << " but the defender stands firm.";
        else oss << midfielder << " floats a ball forward but it's headed away by " << defPlayer << ".";
        return oss.str();
    }

    // ---- PHASE 3 & 4: CHANCE CREATION & RESOLUTION ----
    // The attack has broken through! Now determine what type of chance it is.
    float chanceRoll = randFloat();

    // ---- CHANCE TYPE 1: LONG SHOT (25% of chances) ----
    // Someone has a go from distance (~25 yards). Long shots rarely go in
    // but they're exciting and provide variety in the commentary.
    if (chanceRoll < 0.25f) {
        std::string shooter = (randFloat() < 0.5f) ? midfielder : forward;
        // Shot quality depends on attack strength and the team's willingness to shoot from range
        float shotChance = (atkStr / 20.0f) * 0.3f * attacker.tactics.shootingRangeModifier();

        if (randFloat() < shotChance) {
            // The shot is on target! Now can the keeper save it?
            float saveChance = (gkStr / 20.0f) * 0.7f;
            if (randFloat() < saveChance) {
                // Saved!
                oss << midfielder << " plays it to " << shooter << " who lets fly from 25 yards! ";
                if (randInt(0, 1) == 0) oss << "Good save by the keeper!";
                else oss << "The keeper dives to push it away!";
            } else if (randFloat() < 0.4f) {
                // GOAL from distance! Only 40% of unsaved shots go in (the rest hit post/miss)
                oss << midfielder << " feeds " << shooter << " who strikes it from distance... ";
                oss << "GOAL! What a hit! " << shooter << " makes it "
                    << (isHome ? state.homeGoals + 1 : state.homeGoals) << "-"
                    << (isHome ? state.awayGoals : state.awayGoals + 1) << "!";
                // Update the scorer's goal tally
                for (auto& p : attacker.squad) {
                    if (p.name == shooter) { p.goals++; break; }
                }
            } else {
                // Just over or just wide — close but no cigar
                oss << shooter << " tries his luck from distance... over the bar!";
            }
        } else {
            // Shot was well off target — no danger to the keeper
            oss << midfielder << " has a go from range but it's well wide.";
        }
        return oss.str();
    }

    // ---- CHANCE TYPE 2: THROUGH BALL (30% of chances) ----
    // The midfielder plays a perfectly-weighted ball through the defense
    // to put the forward through on goal. The most dangerous type of chance.
    if (chanceRoll < 0.55f) {
        // Can the midfielder play the through ball? Depends on midfield quality.
        float throughBallChance = (attacker.midfieldStrength() / 20.0f) * 0.6f;

        if (randFloat() < throughBallChance) {
            oss << midfielder << " plays a brilliant through ball to " << forward << "! ";

            float shotPower = (atkStr / 20.0f) * 0.6f;      // Can the striker shoot well?
            float saveChance = (gkStr / 20.0f) * 0.5f;      // Can the keeper save it?

            if (randFloat() < shotPower) {
                // Good shot! Now keeper vs striker...
                if (randFloat() < saveChance) {
                    // Keeper saves it — dramatic!
                    int r = randInt(0, 2);
                    if (r == 0) oss << forward << " is one-on-one... the keeper makes a brilliant save!";
                    else if (r == 1) oss << forward << " shoots but the keeper gets down well to save!";
                    else oss << forward << " rounds the keeper but can only hit the post!";
                } else {
                    // GOAL! Through ball goal — scorer and assister both get credit
                    oss << forward << " is through on goal... he slots it home! ";
                    oss << "GOAL! " << forward << "! "
                        << (isHome ? state.homeGoals + 1 : state.homeGoals) << "-"
                        << (isHome ? state.awayGoals : state.awayGoals + 1) << "!";
                    // Update scorer's goals
                    for (auto& p : attacker.squad) {
                        if (p.name == forward) { p.goals++; break; }
                    }
                    // Update assister's assists
                    for (auto& p : attacker.squad) {
                        if (p.name == midfielder) { p.assists++; break; }
                    }
                }
            } else {
                // Striker fluffs it — poor finishing
                int r = randInt(0, 2);
                if (r == 0) oss << forward << " is in on goal but drags his shot wide!";
                else if (r == 1) oss << forward << " shoots but it's straight at the keeper.";
                else oss << forward << " scuffs his shot and it trickles wide.";
            }
        } else {
            // The through ball was too ambitious — defense cuts it out
            oss << midfielder << " tries to thread a pass to " << forward << " but it's cut out.";
        }
        return oss.str();
    }

    // ---- CHANCE TYPE 3: CROSS (25% of chances) ----
    // A wide player (midfielder) swings a ball into the box.
    // Headers and volleys follow. These can be spectacular goals.
    if (chanceRoll < 0.80f) {
        std::string crosser = pickPlayer(attacker, Position::MID);

        oss << crosser << " swings in a cross... ";

        // Can the forward connect with the cross? Depends on attack strength.
        float headerChance = (atkStr / 20.0f) * 0.35f;

        if (randFloat() < headerChance) {
            // Forward gets on the end of it! Now: save, goal, or miss?
            float saveChance = (gkStr / 20.0f) * 0.45f;

            if (randFloat() < saveChance) {
                // Keeper saves the header
                if (randInt(0, 1) == 0) oss << forward << " meets it with a header but the keeper holds on!";
                else oss << forward << " heads it goalward! Saved!";
            } else if (randFloat() < 0.5f) {
                // GOAL from a cross! 50% of unsaved headers go in.
                int r = randInt(0, 2);
                if (r == 0) oss << forward << " rises highest and heads it in! GOAL! ";
                else if (r == 1) oss << forward << " gets on the end of it and nods it home! GOAL! ";
                else oss << forward << " volleys it first time into the net! GOAL! ";
                oss << (isHome ? state.homeGoals + 1 : state.homeGoals) << "-"
                    << (isHome ? state.awayGoals : state.awayGoals + 1) << "!";
                // Update stats: scorer and crosser (assist)
                for (auto& p : attacker.squad) {
                    if (p.name == forward) { p.goals++; break; }
                }
                for (auto& p : attacker.squad) {
                    if (p.name == crosser) { p.assists++; break; }
                }
            } else {
                // Forward gets his head to it but misses the target
                if (randInt(0, 1) == 0) oss << forward << " gets his head to it but puts it over!";
                else oss << forward << " connects but it flashes just wide!";
            }
        } else {
            // Nobody gets on the end of the cross — defense wins
            int r = randInt(0, 2);
            if (r == 0) oss << defPlayer << " gets there first and heads it clear.";
            else if (r == 1) oss << "nobody can get on the end of it.";
            else oss << "it's too deep and goes out for a goal kick.";
        }
        return oss.str();
    }

    // ---- CHANCE TYPE 4: CORNER KICK (remaining 20% of chances) ----
    // Set piece opportunity. Corners have a fixed 12% goal rate — relatively low
    // but they're free headers so occasionally a defender pops up and scores.
    {
        oss << attacker.name << " win a corner... ";
        std::string taker = pickPlayer(attacker, Position::MID);
        oss << taker << " delivers it... ";

        float setpieceChance = 0.12f;  // 12% chance of scoring from a corner
        if (randFloat() < setpieceChance) {
            // GOAL from a corner! Usually a defender heading it in unmarked.
            std::string scorer = pickPlayer(attacker, Position::DEF);
            oss << scorer << " rises unmarked and heads it in! GOAL! "
                << (isHome ? state.homeGoals + 1 : state.homeGoals) << "-"
                << (isHome ? state.awayGoals : state.awayGoals + 1) << "!";
            for (auto& p : attacker.squad) {
                if (p.name == scorer) { p.goals++; break; }
            }
        } else {
            // Corner is dealt with — no goal
            int r = randInt(0, 2);
            if (r == 0) oss << "cleared by the defense.";
            else if (r == 1) oss << "the keeper comes and claims it.";
            else oss << defPlayer << " heads it away.";
        }
        return oss.str();
    }
}
