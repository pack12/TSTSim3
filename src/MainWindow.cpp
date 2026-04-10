// MainWindow.cpp
// Implementation of the main window — the central controller of the application.
// This file handles:
//   - Building all UI screens in the constructor
//   - Wiring up signal/slot connections
//   - Navigation between screens
//   - Game flow (starting games, playing matches, advancing seasons)
//   - End-of-season logic (aging players, retiring old ones, regenerating squads)

#include "MainWindow.h"
#include "SquadWidget.h"
#include "LeagueTableWidget.h"
#include "TacticsWidget.h"
#include "MatchWidget.h"
#include "TransferWidget.h"
#include "TopScorersWidget.h"
#include "FixturesWidget.h"
#include "PlayerProfileWidget.h"
#include "SaveLoad.h"
#include "DataGen.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStatusBar>
#include <QFont>
#include <QApplication>
#include <random>
#include <algorithm>

// ============================================================================
// CONSTRUCTOR
// ============================================================================
// Builds the entire UI. This is a big constructor because it creates ALL screens upfront
// and adds them to the QStackedWidget. Each screen is a QWidget with its own layout.
//
// The flow after construction:
//   1. Title screen is shown first (showTitleScreen())
//   2. Player clicks "New Game" → league is initialized → team selection screen shown
//   3. Player picks a team → main menu is shown
//   4. From the main menu, player navigates to sub-screens and back
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("TST3 Soccer Manager");
    resize(1000, 700);  // Default window size — big enough to see everything

    // QStackedWidget: the container that holds all screens. Only one is visible at a time.
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    // ========== TITLE SCREEN ==========
    // The first thing the player sees: game title, subtitle, and three big buttons.
    titlePage = new QWidget;
    auto* titleLayout = new QVBoxLayout(titlePage);
    titleLayout->setAlignment(Qt::AlignCenter);

    auto* titleLabel = new QLabel("TST3 SOCCER MANAGER");
    QFont titleFont("Helvetica", 28, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLayout->addWidget(titleLabel);

    auto* subtitleLabel = new QLabel("A Text Simulation Football Manager");
    subtitleLabel->setFont(QFont("Helvetica", 14));
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #666;");  // Grey subtitle text
    titleLayout->addWidget(subtitleLabel);
    titleLayout->addSpacing(30);  // Gap between subtitle and buttons

    // Three buttons: New Game, Load Game, Quit — all the same size and centered
    auto* newGameBtn = new QPushButton("New Game");
    newGameBtn->setFixedSize(200, 40);
    newGameBtn->setFont(QFont("Helvetica", 14));
    titleLayout->addWidget(newGameBtn, 0, Qt::AlignCenter);

    auto* loadGameBtn = new QPushButton("Load Game");
    loadGameBtn->setFixedSize(200, 40);
    loadGameBtn->setFont(QFont("Helvetica", 14));
    titleLayout->addWidget(loadGameBtn, 0, Qt::AlignCenter);

    auto* quitBtn = new QPushButton("Quit");
    quitBtn->setFixedSize(200, 40);
    quitBtn->setFont(QFont("Helvetica", 14));
    titleLayout->addWidget(quitBtn, 0, Qt::AlignCenter);

    // Connect button clicks to their handler functions
    connect(newGameBtn, &QPushButton::clicked, this, &MainWindow::onNewGame);
    connect(loadGameBtn, &QPushButton::clicked, this, &MainWindow::onLoadGame);
    connect(quitBtn, &QPushButton::clicked, qApp, &QApplication::quit);

    stack->addWidget(titlePage);

    // ========== TEAM SELECTION SCREEN ==========
    // Shows all 16 teams in a list with their overall rating and budget.
    // Player can single-click to highlight, then click "Select Team" or double-click.
    teamSelectPage = new QWidget;
    auto* tsLayout = new QVBoxLayout(teamSelectPage);
    auto* tsTitle = new QLabel("Choose Your Team");
    tsTitle->setFont(QFont("Helvetica", 20, QFont::Bold));
    tsTitle->setAlignment(Qt::AlignCenter);
    tsLayout->addWidget(tsTitle);

    teamList = new QListWidget;
    teamList->setFont(QFont("Courier", 13));           // Monospace so columns line up
    teamList->setAlternatingRowColors(true);            // Zebra striping for readability
    tsLayout->addWidget(teamList);

    auto* selectBtn = new QPushButton("Select Team");
    selectBtn->setFixedHeight(36);
    selectBtn->setFont(QFont("Helvetica", 13));
    tsLayout->addWidget(selectBtn);
    // Two ways to select: click the button, or double-click a team in the list
    connect(selectBtn, &QPushButton::clicked, this, [this]() {
        int row = teamList->currentRow();
        if (row >= 0) onTeamSelected(row);
    });
    connect(teamList, &QListWidget::itemDoubleClicked, this, [this]() {
        int row = teamList->currentRow();
        if (row >= 0) onTeamSelected(row);
    });

    stack->addWidget(teamSelectPage);

    // ========== MAIN MENU (HOME SCREEN) ==========
    // Shows your team info at the top, then a vertical list of action buttons.
    menuPage = new QWidget;
    auto* menuLayout = new QVBoxLayout(menuPage);

    // Team info label at the top: shows team name, season, budget, formation, league position
    teamInfoLabel = new QLabel;
    teamInfoLabel->setFont(QFont("Helvetica", 14));
    teamInfoLabel->setWordWrap(true);
    menuLayout->addWidget(teamInfoLabel);

    menuLayout->addSpacing(10);

    // Build the menu buttons using an array of {text, handler} pairs.
    // This is cleaner than writing each button individually.
    struct MenuBtn { QString text; void (MainWindow::*slot)(); };
    MenuBtn buttons[] = {
        {"Play Next Match",  &MainWindow::onPlayMatch},
        {"View Squad",       &MainWindow::onViewSquad},
        {"Tactics",          &MainWindow::onViewTactics},
        {"League Table",     &MainWindow::onViewLeagueTable},
        {"Fixtures & Results", &MainWindow::onViewFixtures},
        {"Transfer Market",  &MainWindow::onViewTransferMarket},
        {"Top Scorers",      &MainWindow::onViewTopScorers},
        {"Save Game",        &MainWindow::onSaveGame},
    };

    auto* btnGrid = new QVBoxLayout;
    for (auto& b : buttons) {
        auto* btn = new QPushButton(b.text);
        btn->setFixedHeight(36);
        btn->setFont(QFont("Helvetica", 13));
        btnGrid->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, b.slot);
    }
    menuLayout->addLayout(btnGrid);
    menuLayout->addStretch();  // Push everything up, leave empty space at the bottom

    stack->addWidget(menuPage);

    // ========== SUB-WIDGETS ==========
    // Each sub-screen is a custom widget that's created once and reused.
    // When navigating to a screen, we call its refresh() method with current data,
    // then switch the QStackedWidget to show it.
    // Each widget emits a backClicked() signal that we connect to onBackToMenu().

    squadWidget = new SquadWidget;
    connect(squadWidget, &SquadWidget::backClicked, this, &MainWindow::onBackToMenu);
    connect(squadWidget, &SquadWidget::playerSelected, this, &MainWindow::onPlayerSelected);
    stack->addWidget(squadWidget);

    // Player profile: when "Back to Squad" is clicked, refresh the squad and go back to it
    playerProfileWidget = new PlayerProfileWidget;
    connect(playerProfileWidget, &PlayerProfileWidget::backClicked, this, [this]() {
        squadWidget->refresh(league.teams[playerTeamIdx]);
        stack->setCurrentWidget(squadWidget);
    });
    stack->addWidget(playerProfileWidget);

    leagueTableWidget = new LeagueTableWidget;
    connect(leagueTableWidget, &LeagueTableWidget::backClicked, this, &MainWindow::onBackToMenu);
    stack->addWidget(leagueTableWidget);

    tacticsWidget = new TacticsWidget;
    connect(tacticsWidget, &TacticsWidget::backClicked, this, &MainWindow::onBackToMenu);
    stack->addWidget(tacticsWidget);

    matchWidget = new MatchWidget;
    connect(matchWidget, &MatchWidget::matchFinished, this, &MainWindow::onMatchFinished);
    stack->addWidget(matchWidget);

    transferWidget = new TransferWidget;
    connect(transferWidget, &TransferWidget::backClicked, this, &MainWindow::onBackToMenu);
    stack->addWidget(transferWidget);

    topScorersWidget = new TopScorersWidget;
    connect(topScorersWidget, &TopScorersWidget::backClicked, this, &MainWindow::onBackToMenu);
    stack->addWidget(topScorersWidget);

    fixturesWidget = new FixturesWidget;
    connect(fixturesWidget, &FixturesWidget::backClicked, this, &MainWindow::onBackToMenu);
    stack->addWidget(fixturesWidget);

    // Start on the title screen
    showTitleScreen();
}

// Switches the visible page to the title screen.
void MainWindow::showTitleScreen() {
    stack->setCurrentWidget(titlePage);
}

// ============================================================================
// NEW GAME
// ============================================================================
// Creates a fresh league (16 teams, blank table, new schedule) and shows team selection.
void MainWindow::onNewGame() {
    league.initialize();
    showTeamSelect();
}

// ============================================================================
// LOAD GAME
// ============================================================================
// Attempts to load a saved game from the default save file.
// If successful, goes straight to the main menu. If not, shows an info dialog.
void MainWindow::onLoadGame() {
    if (SaveLoad::loadGame(league, playerTeamIdx, SaveLoad::defaultSavePath())) {
        market.generateListings(league, playerTeamIdx);
        showMainMenu();
    } else {
        QMessageBox::information(this, "Load Game", "No save file found.");
    }
}

// ============================================================================
// SHOW TEAM SELECT
// ============================================================================
// Populates the team list and switches to the team selection screen.
// Each row shows: rank, team name, average overall rating, and budget.
void MainWindow::showTeamSelect() {
    teamList->clear();
    for (int i = 0; i < (int)league.teams.size(); i++) {
        auto& team = league.teams[i];
        // Format: "  1.  Alderwick City            OVR: 14   Budget: £10000k"
        QString entry = QString("%1.  %2    OVR: %3   Budget: £%4k")
            .arg(i + 1, 2)                                          // Right-padded rank number
            .arg(QString::fromStdString(team.name), -22)            // Left-aligned team name (22 chars)
            .arg(team.averageOverall(), 0, 'f', 0)                  // Overall rating, no decimal
            .arg(team.budget / 1000);                                // Budget in thousands
        teamList->addItem(entry);
    }
    teamList->setCurrentRow(0);  // Pre-select the first team
    stack->setCurrentWidget(teamSelectPage);
}

// ============================================================================
// ON TEAM SELECTED
// ============================================================================
// Called when the player picks a team. Stores their choice, generates the initial
// transfer market, shows a welcome message, and navigates to the main menu.
void MainWindow::onTeamSelected(int index) {
    playerTeamIdx = index;
    market.generateListings(league, playerTeamIdx);

    // Welcome popup with the team's name, budget, and squad size
    QMessageBox::information(this, "Welcome!",
        QString("You are now the manager of %1!\n\nBudget: £%2k\nSquad size: %3 players")
            .arg(QString::fromStdString(league.teams[playerTeamIdx].name))
            .arg(league.teams[playerTeamIdx].budget / 1000)
            .arg(league.teams[playerTeamIdx].squad.size()));

    showMainMenu();
}

// ============================================================================
// SHOW MAIN MENU
// ============================================================================
// Updates the team info display and switches to the main menu screen.
// The info label shows the team name, season, week, budget, formation, play style,
// and current league position.
void MainWindow::showMainMenu() {
    updateStatusBar();

    auto& team = league.teams[playerTeamIdx];

    // Find the player's position in the league table
    int leaguePos = 0;
    int pts = 0;
    for (int i = 0; i < (int)league.table.size(); i++) {
        if (league.table[i].teamIdx == playerTeamIdx) {
            leaguePos = i + 1;  // 1-indexed for display
            pts = league.table[i].points();
            break;
        }
    }

    // Build HTML-formatted team info string
    QString info = QString(
        "<h2>%1</h2>"
        "<p><b>Season %2</b> — Week %3 of %4</p>"
        "<p>Budget: <b>£%5k</b> | Formation: <b>%6</b> | "
        "Style: <b>%7</b></p>"
        "<p>League Position: <b>%8 / %9</b> (%10 pts)</p>")
        .arg(QString::fromStdString(team.name))
        .arg(league.season)
        .arg(league.seasonComplete() ? league.totalWeeks() : league.currentWeek + 1)
        .arg(league.totalWeeks())
        .arg(team.budget / 1000)
        .arg(QString::fromStdString(team.tactics.formationStr()))
        .arg(QString::fromStdString(team.tactics.playStyleStr()))
        .arg(leaguePos).arg(league.teams.size())
        .arg(pts);

    teamInfoLabel->setText(info);
    stack->setCurrentWidget(menuPage);
}

// Updates the status bar at the bottom of the window with team name, season, and week.
void MainWindow::updateStatusBar() {
    auto& team = league.teams[playerTeamIdx];
    statusBar()->showMessage(
        QString("%1 | Season %2 | Week %3/%4")
            .arg(QString::fromStdString(team.name))
            .arg(league.season)
            .arg(league.seasonComplete() ? league.totalWeeks() : league.currentWeek + 1)
            .arg(league.totalWeeks()));
}

// ============================================================================
// PLAY MATCH
// ============================================================================
// Simulates all AI matches for the current week instantly, then starts the player's
// match with live commentary on the match screen.
//
// Flow:
//   1. If the season is complete, trigger end-of-season instead
//   2. Find which fixture involves the player's team
//   3. Simulate all OTHER fixtures instantly (no commentary needed)
//   4. Start the player's match on the MatchWidget with live event playback
//   5. Advance the week counter
void MainWindow::onPlayMatch() {
    if (league.seasonComplete()) {
        onEndOfSeason();
        return;
    }

    auto& fixtures = league.currentFixtures();

    // Find the player's fixture
    int playerFixtureIdx = -1;
    for (int i = 0; i < (int)fixtures.size(); i++) {
        if (fixtures[i].homeTeamIdx == playerTeamIdx || fixtures[i].awayTeamIdx == playerTeamIdx) {
            playerFixtureIdx = i;
        }
    }

    // Simulate all AI matches (non-player fixtures) instantly
    for (int i = 0; i < (int)fixtures.size(); i++) {
        if (i == playerFixtureIdx) continue;  // Skip the player's match
        auto& f = fixtures[i];
        f.result = engine.simulate(league.teams[f.homeTeamIdx], league.teams[f.awayTeamIdx]);
        f.played = true;
        league.updateTable(f);  // Immediately update the table for AI matches
    }

    // Start the player's match with live commentary
    if (playerFixtureIdx >= 0) {
        auto& f = fixtures[playerFixtureIdx];
        matchWidget->startMatch(league, engine, f, playerFixtureIdx);
        stack->setCurrentWidget(matchWidget);
    }

    league.currentWeek++;  // Advance to the next week
}

// ============================================================================
// ON MATCH FINISHED
// ============================================================================
// Called when the player clicks "Continue" after their match ends.
// Updates the league table for the player's match result, then returns to the main menu.
// (AI matches were already updated in onPlayMatch.)
void MainWindow::onMatchFinished() {
    int prevWeek = league.currentWeek - 1;
    if (prevWeek >= 0 && prevWeek < (int)league.schedule.size()) {
        for (auto& f : league.schedule[prevWeek]) {
            if ((f.homeTeamIdx == playerTeamIdx || f.awayTeamIdx == playerTeamIdx) && f.played) {
                league.updateTable(f);
                break;
            }
        }
    }
    showMainMenu();
}

// ============================================================================
// NAVIGATION HANDLERS
// ============================================================================
// Each of these refreshes the target widget with current data, then switches to it.

void MainWindow::onViewSquad() {
    squadWidget->refresh(league.teams[playerTeamIdx]);
    stack->setCurrentWidget(squadWidget);
}

void MainWindow::onViewTactics() {
    tacticsWidget->refresh(league.teams[playerTeamIdx]);
    stack->setCurrentWidget(tacticsWidget);
}

void MainWindow::onViewLeagueTable() {
    leagueTableWidget->refresh(league);
    stack->setCurrentWidget(leagueTableWidget);
}

void MainWindow::onViewFixtures() {
    fixturesWidget->refresh(league, playerTeamIdx);
    stack->setCurrentWidget(fixturesWidget);
}

void MainWindow::onViewTransferMarket() {
    transferWidget->refresh(league, market, playerTeamIdx);
    stack->setCurrentWidget(transferWidget);
}

void MainWindow::onViewTopScorers() {
    topScorersWidget->refresh(league);
    stack->setCurrentWidget(topScorersWidget);
}

void MainWindow::onSaveGame() {
    if (SaveLoad::saveGame(league, playerTeamIdx, SaveLoad::defaultSavePath())) {
        QMessageBox::information(this, "Save", "Game saved successfully!");
    } else {
        QMessageBox::warning(this, "Save", "Error saving game.");
    }
}

// Goes back to the main menu from any sub-screen.
void MainWindow::onBackToMenu() {
    showMainMenu();
}

// ============================================================================
// ON PLAYER SELECTED
// ============================================================================
// Called when the player double-clicks a player in the squad list.
// Checks if the selected player is in the starting XI, then shows their profile.
void MainWindow::onPlayerSelected(int squadIndex) {
    auto& team = league.teams[playerTeamIdx];
    if (squadIndex < 0 || squadIndex >= (int)team.squad.size()) return;

    // Check if this player is in the starting XI
    auto xi = team.getStartingEleven();
    bool isStarter = std::find(xi.begin(), xi.end(), squadIndex) != xi.end();

    playerProfileWidget->refresh(team.squad[squadIndex], team.name, isStarter);
    stack->setCurrentWidget(playerProfileWidget);
}

// ============================================================================
// END OF SEASON
// ============================================================================
// Called when all 30 weeks have been played. Handles the transition to the next season.
//
// Steps:
//   1. Show who won the league and where the player finished
//   2. Ask if the player wants to continue to the next season
//   3. If yes:
//      a. Increment the season counter
//      b. Age all players by 1 year
//      c. Reset all season stats (goals, assists, appearances) to 0
//      d. Retire any players over 36 (remove them from squads)
//      e. If any team has fewer than 16 players (after retirements), generate
//         new young players to fill the gaps (youth academy regen)
//      f. Reset the league table to all zeros
//      g. Generate a fresh schedule for the new season
//      h. Refresh the transfer market
void MainWindow::onEndOfSeason() {
    // Find the champion and the player's finishing position
    QString champion = QString::fromStdString(league.teams[league.table[0].teamIdx].name);
    int playerPos = 0;
    for (int i = 0; i < (int)league.table.size(); i++) {
        if (league.table[i].teamIdx == playerTeamIdx) { playerPos = i + 1; break; }
    }

    // Build the end-of-season message
    QString msg = QString("Season %1 Complete!\n\nChampions: %2\nYour finish: %3")
        .arg(league.season).arg(champion).arg(playerPos);
    if (playerPos == 1) msg += "\n\nCONGRATULATIONS - YOU ARE CHAMPIONS!";

    auto reply = QMessageBox::question(this, "End of Season", msg + "\n\nContinue to next season?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        league.season++;
        league.currentWeek = 0;

        std::mt19937 gen(std::random_device{}());

        for (auto& team : league.teams) {
            // Reset stats and age all players
            for (auto& p : team.squad) {
                p.goals = 0; p.assists = 0; p.appearances = 0; p.age++;
            }
            // Retire players over 36 — remove them from the squad entirely.
            // Uses the erase-remove idiom: remove_if moves retirees to the end,
            // then erase chops them off.
            team.squad.erase(
                std::remove_if(team.squad.begin(), team.squad.end(),
                    [](const Player& p) { return p.age > 36; }),
                team.squad.end());

            // Youth academy regen: if the squad is too small after retirements,
            // generate new players to bring it back up to at least 16.
            // New players are slightly below the team average (quality -3 to +1),
            // representing young academy graduates who aren't quite first-team level yet.
            while ((int)team.squad.size() < 16) {
                Position pos;
                int r = std::uniform_int_distribution<>(0, 3)(gen);
                switch (r) {
                    case 0: pos = Position::DEF; break;
                    case 1: pos = Position::MID; break;
                    case 2: pos = Position::FWD; break;
                    default: pos = Position::GK; break;
                }
                int q = std::clamp((int)team.averageOverall() +
                    std::uniform_int_distribution<>(-3, 1)(gen), 4, 18);
                team.squad.push_back(Player::generate(pos, q, DataGen::generatePlayerName(gen)));
            }
        }

        // Reset the league table to all zeros for the new season
        league.table.clear();
        for (int i = 0; i < (int)league.teams.size(); i++) {
            LeagueEntry entry; entry.teamIdx = i; league.table.push_back(entry);
        }
        league.generateSchedule();  // New schedule for the new season
        market.refreshListings(league, playerTeamIdx);  // Fresh transfer market
        showMainMenu();
    }
}
