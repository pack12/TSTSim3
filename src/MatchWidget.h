// MatchWidget.h
// The live match screen — shows your match being played with real-time commentary.
//
// HOW IT WORKS:
// The match is simulated ALL AT ONCE by the MatchEngine (not minute-by-minute in real time).
// The result and all events are generated upfront. Then a QTimer replays the events
// one at a time with delays to create the illusion of a live match.
//
// Normal events: displayed every 600ms (fast enough to keep moving, slow enough to read)
// Goals: displayed for 1500ms (they deserve more screen time!)
//
// The score at the top updates in real time as goals "happen" (are displayed).
// At full time, the other league results are shown and a "Continue" button appears.

#pragma once
#include "League.h"
#include "MatchEngine.h"
#include <QWidget>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

class MatchWidget : public QWidget {
    Q_OBJECT
public:
    explicit MatchWidget(QWidget* parent = nullptr);

    // Simulates the match and starts the commentary replay.
    // The match is fully simulated immediately; the QTimer then reveals events one by one.
    void startMatch(League& league, MatchEngine& engine, Fixture& fixture, int fixtureIdx);

signals:
    void matchFinished();  // Emitted when the player clicks "Continue" after full time

private slots:
    void showNextEvent();  // Called by the QTimer to display the next commentary event
    void onContinue();     // Called when the "Continue" button is clicked

private:
    QLabel* scoreLabel = nullptr;           // "Alderwick City  2 - 1  Glendale Rovers" at the top
    QTextEdit* commentaryBox = nullptr;     // Scrolling text area for match commentary
    QPushButton* continueBtn = nullptr;     // "Continue" button — only visible after full time
    QLabel* otherResultsLabel = nullptr;    // Other league results — shown after full time
    QTimer* eventTimer = nullptr;           // Timer that fires to show the next event

    MatchResult result;                     // The pre-simulated match result (all events stored here)
    int currentEvent = 0;                   // Which event we're about to show next
    Fixture* currentFixture = nullptr;      // Pointer to the fixture being played
    League* currentLeague = nullptr;        // Pointer to the league (for showing other results)
};
