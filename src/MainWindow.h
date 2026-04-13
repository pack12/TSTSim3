// MainWindow.h
// The main window — restructured with a Football Manager-style layout:
//
// LAYOUT STRUCTURE:
//   outerStack (QStackedWidget — switches between pre-game and in-game views)
//   ├── titlePage        (full-screen: New Game / Load Game / Quit)
//   ├── teamSelectPage   (full-screen: pick your team)
//   └── gamePage          (the main gameplay container)
//       └── QVBoxLayout
//           ├── topBar    (persistent header: team name, season, week, budget, position)
//           └── QHBoxLayout
//               ├── sidebar       (left nav: Home, Squad, Tactics, etc.)
//               └── contentStack  (right: the actual page content)
//
// The sidebar is always visible during gameplay, replacing the old button-list menu.
// Clicking a sidebar item highlights it with an accent green bar and switches content.
// The top bar always shows your team context so you never lose track of where you are.

#pragma once
#include "League.h"
#include "MatchEngine.h"
#include "TransferMarket.h"
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>

class SquadWidget;
class LeagueTableWidget;
class TacticsWidget;
class MatchWidget;
class TransferWidget;
class TopScorersWidget;
class FixturesWidget;
class PlayerProfileWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

public slots:
    // FM-style navigation: any widget can request navigation to a team or player
    void navigateToTeam(int teamIdx);
    void navigateToPlayer(int teamIdx, int squadIdx);

private slots:
    void onNewGame();
    void onLoadGame();
    void onTeamSelected(int index);
    void onPlayMatch();
    void onSaveGame();
    void onMatchFinished();
    void onEndOfSeason();
    void onPlayerSelected(int squadIndex);

private:
    // Navigation: switches content page and highlights the active sidebar button
    static const int NUM_NAV_BTNS = 7;
    void navigateTo(int pageIndex);
    void updateTopBar();
    void updateDashboard();
    void setSidebarEnabled(bool enabled);

    void showTitleScreen();
    void showTeamSelect();
    void showGameUI();

    // ---- OUTER STRUCTURE ----
    QStackedWidget* outerStack = nullptr;   // Title / TeamSelect / GamePage
    QWidget* titlePage = nullptr;
    QWidget* teamSelectPage = nullptr;
    QListWidget* teamList = nullptr;
    QWidget* gamePage = nullptr;

    // ---- TOP BAR ----
    QLabel* topBarTeamName = nullptr;
    QLabel* topBarInfo = nullptr;

    // ---- SIDEBAR ----
    QWidget* sidebar = nullptr;
    QPushButton* navBtns[NUM_NAV_BTNS] = {};     // Home, Squad, Tactics, League, Fixtures, Transfers, Scorers
    QWidget* navPages[NUM_NAV_BTNS] = {};         // The content widget each button maps to
    QPushButton* playMatchBtn = nullptr;
    QPushButton* saveGameBtn = nullptr;

    // ---- CONTENT AREA ----
    QStackedWidget* contentStack = nullptr;
    QWidget* dashboardPage = nullptr;
    QLabel* dashboardLabel = nullptr;
    QLabel* vsLabel = nullptr;
    QLabel* homeOrAwayLabel = nullptr;
    QLabel* nextMatchHeading = nullptr;
    QPushButton* nextMatchBtn = nullptr;      // Opponent name — hover-reveal link
    int nextMatchOpponentIdx = -1;            // Team index of the next opponent

    // ---- SUB-PAGE WIDGETS ----
    SquadWidget* squadWidget = nullptr;
    LeagueTableWidget* leagueTableWidget = nullptr;
    TacticsWidget* tacticsWidget = nullptr;
    MatchWidget* matchWidget = nullptr;
    TransferWidget* transferWidget = nullptr;
    TopScorersWidget* topScorersWidget = nullptr;
    FixturesWidget* fixturesWidget = nullptr;
    PlayerProfileWidget* playerProfileWidget = nullptr;

    // ---- GAME STATE ----
    League league;
    MatchEngine engine;
    TransferMarket market;
    int playerTeamIdx = 0;
    int viewingTeamIdx = -1;  // Which team is shown in squad view (-1 = player's own team)
};
