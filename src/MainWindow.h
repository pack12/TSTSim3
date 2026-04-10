// MainWindow.h
// The main window of the application — the "hub" that holds everything together.
//
// MainWindow uses a QStackedWidget to switch between different screens:
//   - Title screen (New Game / Load Game / Quit)
//   - Team selection screen (pick which of the 16 teams to manage)
//   - Main menu (the "home screen" — links to all other screens)
//   - Squad, Tactics, League Table, Fixtures, Transfer Market, Top Scorers, Player Profile
//   - Match screen (live commentary during your match)
//
// It also owns the core game state: the League, MatchEngine, TransferMarket, and
// which team the player is controlling. When the player navigates to a sub-screen,
// MainWindow passes the relevant data to that widget's refresh() method.
//
// Qt's signal/slot system connects everything: clicking "View Squad" triggers
// onViewSquad(), which refreshes the squad widget and switches to it.
// The "Back" button on each sub-screen emits a backClicked() signal that
// MainWindow listens for and responds to by switching back to the main menu.

#pragma once
#include "League.h"
#include "MatchEngine.h"
#include "TransferMarket.h"
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>

// Forward declarations — these classes are defined in their own header files.
// We only need pointers to them here, so forward declarations avoid
// #including every widget header in this file (faster compilation).
class SquadWidget;
class LeagueTableWidget;
class TacticsWidget;
class MatchWidget;
class TransferWidget;
class TopScorersWidget;
class FixturesWidget;
class PlayerProfileWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT  // Required for any class that uses Qt signals and slots
public:
    explicit MainWindow(QWidget* parent = nullptr);

// ---- SLOTS ----
// These are functions that get called in response to user actions (button clicks, etc.)
// Qt's signal/slot system connects a signal (e.g. button::clicked) to a slot (e.g. onNewGame).
private slots:
    void onNewGame();           // "New Game" button on title screen
    void onLoadGame();          // "Load Game" button on title screen
    void onTeamSelected(int index);  // Player picks a team from the list
    void onPlayMatch();         // "Play Next Match" from the main menu
    void onViewSquad();         // "View Squad" from the main menu
    void onViewTactics();       // "Tactics" from the main menu
    void onViewLeagueTable();   // "League Table" from the main menu
    void onViewFixtures();      // "Fixtures & Results" from the main menu
    void onViewTransferMarket(); // "Transfer Market" from the main menu
    void onViewTopScorers();    // "Top Scorers" from the main menu
    void onSaveGame();          // "Save Game" from the main menu
    void onMatchFinished();     // Called when the match commentary finishes and player clicks "Continue"
    void onBackToMenu();        // "Back" button on any sub-screen
    void onEndOfSeason();       // Called when all 30 weeks are done — handles season transition
    void onPlayerSelected(int squadIndex);  // Player double-clicks a player in the squad list

private:
    void showTitleScreen();     // Switch to the title/splash screen
    void showTeamSelect();      // Switch to the team selection screen
    void showMainMenu();        // Switch to the main menu (home screen)
    void updateStatusBar();     // Update the status bar with team name, season, and week

    // QStackedWidget is like a deck of cards — you can only see one "card" (widget) at a time.
    // Calling stack->setCurrentWidget(someWidget) switches which screen is visible.
    QStackedWidget* stack = nullptr;

    // ---- SCREEN WIDGETS ----
    // Title screen: New Game / Load Game / Quit buttons
    QWidget* titlePage = nullptr;

    // Team selection screen: list of all 16 teams with quality and budget info
    QWidget* teamSelectPage = nullptr;
    QListWidget* teamList = nullptr;      // The scrollable list of teams to choose from

    // Main menu: shows team info at the top, then buttons for each action
    QWidget* menuPage = nullptr;
    QLabel* teamInfoLabel = nullptr;      // Displays team name, budget, formation, league position

    // ---- SUB-PAGE WIDGETS ----
    // Each of these is a custom widget class defined in its own .h/.cpp files.
    // They're created once in the constructor and reused by calling refresh() with new data.
    SquadWidget* squadWidget = nullptr;
    LeagueTableWidget* leagueTableWidget = nullptr;
    TacticsWidget* tacticsWidget = nullptr;
    MatchWidget* matchWidget = nullptr;
    TransferWidget* transferWidget = nullptr;
    TopScorersWidget* topScorersWidget = nullptr;
    FixturesWidget* fixturesWidget = nullptr;
    PlayerProfileWidget* playerProfileWidget = nullptr;

    // ---- GAME STATE ----
    // These are the core data objects that represent the entire game.
    // They persist for the lifetime of the application.
    League league;              // The league: 16 teams, schedule, table
    MatchEngine engine;         // The match simulator
    TransferMarket market;      // Available players for purchase
    int playerTeamIdx = 0;      // Which team the player is managing (index into league.teams[])
};
