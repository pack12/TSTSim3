// MatchWidget.cpp
// Implementation of the live match commentary screen.
//
// REPLAY SYSTEM:
// The match is simulated in one call to engine.simulate(), which returns a MatchResult
// containing all events. We then use a QTimer to reveal events one by one, creating
// the feel of a live match. The timer interval changes based on the event type:
//   - Normal event: 600ms delay (keep it moving)
//   - Goal event: 1500ms delay (dramatic pause!)
//
// The score updates after each event, counting goals as they appear. At full time,
// we show "FULL TIME!", display the final score again, show other league results,
// and reveal the "Continue" button.

#include "MatchWidget.h"
#include <QVBoxLayout>
#include <QScrollBar>

MatchWidget::MatchWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    // Score label at the top — big and bold
    scoreLabel = new QLabel;
    scoreLabel->setFont(QFont("Helvetica", 20, QFont::Bold));
    scoreLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(scoreLabel);

    // Commentary box — read-only text area that accumulates match events
    commentaryBox = new QTextEdit;
    commentaryBox->setReadOnly(true);
    commentaryBox->setFont(QFont("Courier", 12));
    layout->addWidget(commentaryBox);

    // Other results label — hidden until full time
    otherResultsLabel = new QLabel;
    otherResultsLabel->setFont(QFont("Courier", 11));
    otherResultsLabel->setWordWrap(true);
    otherResultsLabel->setVisible(false);
    layout->addWidget(otherResultsLabel);

    // Continue button — hidden until full time
    continueBtn = new QPushButton("Continue");
    continueBtn->setFixedHeight(36);
    continueBtn->setVisible(false);
    connect(continueBtn, &QPushButton::clicked, this, &MatchWidget::onContinue);
    layout->addWidget(continueBtn);

    // Timer that fires to show the next event
    eventTimer = new QTimer(this);
    connect(eventTimer, &QTimer::timeout, this, &MatchWidget::showNextEvent);
}

// ============================================================================
// START MATCH
// ============================================================================
// Simulates the match and begins the commentary replay.
// The entire match is simulated upfront — the QTimer just reveals events over time.
void MatchWidget::startMatch(League& league, MatchEngine& engine, Fixture& fixture, int /*fixtureIdx*/) {
    currentFixture = &fixture;
    currentLeague = &league;
    currentEvent = 0;
    commentaryBox->clear();
    continueBtn->setVisible(false);
    otherResultsLabel->setVisible(false);

    auto& homeTeam = league.teams[fixture.homeTeamIdx];
    auto& awayTeam = league.teams[fixture.awayTeamIdx];

    // Show initial score: 0 - 0
    scoreLabel->setText(QString("%1  0 - 0  %2")
        .arg(QString::fromStdString(homeTeam.name))
        .arg(QString::fromStdString(awayTeam.name)));

    // Simulate the ENTIRE match at once (not interactive — we replay events ourselves)
    result = engine.simulate(homeTeam, awayTeam, false);

    commentaryBox->append("<b>KICK OFF!</b>\n");

    // Start the event replay timer (or show full time immediately if no events)
    if (!result.events.empty()) {
        eventTimer->start(600);  // First event in 600ms
    } else {
        // Incredibly boring 0-0 with zero events — skip straight to full time
        scoreLabel->setText(QString("%1  %2 - %3  %4")
            .arg(QString::fromStdString(result.homeTeam))
            .arg(result.homeGoals).arg(result.awayGoals)
            .arg(QString::fromStdString(result.awayTeam)));
        commentaryBox->append("\n<b>FULL TIME!</b>");
        continueBtn->setVisible(true);
    }
}

// ============================================================================
// SHOW NEXT EVENT
// ============================================================================
// Called by the QTimer. Displays the next event from the pre-simulated match result.
//
// When all events have been shown:
//   1. Stop the timer
//   2. Show "FULL TIME!" with the final score
//   3. Display other league results from the same week
//   4. Show the "Continue" button
void MatchWidget::showNextEvent() {
    if (currentEvent >= (int)result.events.size()) {
        eventTimer->stop();

        // Show final score
        scoreLabel->setText(QString("%1  %2 - %3  %4")
            .arg(QString::fromStdString(result.homeTeam))
            .arg(result.homeGoals).arg(result.awayGoals)
            .arg(QString::fromStdString(result.awayTeam)));

        commentaryBox->append("\n<b>FULL TIME!</b>");
        commentaryBox->append(QString("<b>%1  %2 - %3  %4</b>")
            .arg(QString::fromStdString(result.homeTeam))
            .arg(result.homeGoals).arg(result.awayGoals)
            .arg(QString::fromStdString(result.awayTeam)));

        // Show other results from this week's fixtures
        if (currentLeague) {
            QString others = "\nOther Results:\n";
            int prevWeek = currentLeague->currentWeek;
            if (prevWeek < (int)currentLeague->schedule.size()) {
                for (auto& f : currentLeague->schedule[prevWeek]) {
                    if (&f == currentFixture) continue;  // Skip our own match
                    if (f.played) {
                        others += QString("%1  %2 - %3  %4\n")
                            .arg(QString::fromStdString(currentLeague->teams[f.homeTeamIdx].name), -22)
                            .arg(f.result.homeGoals).arg(f.result.awayGoals)
                            .arg(QString::fromStdString(currentLeague->teams[f.awayTeamIdx].name));
                    }
                }
            }
            otherResultsLabel->setText(others);
            otherResultsLabel->setVisible(true);
        }

        continueBtn->setVisible(true);
        return;
    }

    // Display the current event
    auto& event = result.events[currentEvent];

    // Calculate the running score up to and including this event.
    // We need this because the score display should update as goals "happen".
    int runningHome = 0, runningAway = 0;
    for (int i = 0; i <= currentEvent; i++) {
        if (result.events[i].isGoal) {
            if (result.events[i].isHomeTeam) runningHome++;
            else runningAway++;
        }
    }

    // Format the commentary line — goals get special red bold treatment
    QString line;
    if (event.isGoal) {
        // GOAL! Red, bold, larger font, with a football emoji
        line = QString("<br><b style='color: #2dcc73; font-size: 14px;'>⚽ %1'  %2</b><br>")
            .arg(event.minute)
            .arg(QString::fromStdString(event.description));
        eventTimer->setInterval(2500);  // Pause longer on goals for dramatic effect
    } else {
        // Normal event: plain text with minute number
        line = QString("%1'  %2")
            .arg(event.minute)
            .arg(QString::fromStdString(event.description));
        eventTimer->setInterval(1000);  // Normal pace for regular events
    }

    // Add the line to the commentary box and auto-scroll to the bottom
    commentaryBox->append(line);
    commentaryBox->verticalScrollBar()->setValue(commentaryBox->verticalScrollBar()->maximum());

    // Update the score display
    scoreLabel->setText(QString("%1  %2 - %3  %4")
        .arg(QString::fromStdString(result.homeTeam))
        .arg(runningHome).arg(runningAway)
        .arg(QString::fromStdString(result.awayTeam)));

    currentEvent++;
}

// ============================================================================
// ON CONTINUE
// ============================================================================
// Called when the player clicks "Continue" after the match ends.
// Saves the match result into the fixture object (so it persists in the league data)
// and emits matchFinished() so MainWindow can update the league table.
void MatchWidget::onContinue() {
    if (currentFixture) {
        currentFixture->result = result;
        currentFixture->played = true;
    }
    emit matchFinished();
}
