// MainWindow.cpp
// Implementation of the FM-style main window with persistent sidebar navigation.

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
#include <QStyle>
#include <random>
#include <algorithm>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("TST3 Soccer Manager");
    resize(1100, 750);

    outerStack = new QStackedWidget(this);
    setCentralWidget(outerStack);

    // ========================================================================
    // TITLE SCREEN
    // ========================================================================
    titlePage = new QWidget;
    auto* titleLayout = new QVBoxLayout(titlePage);
    titleLayout->setAlignment(Qt::AlignCenter);

    auto* titleLabel = new QLabel("TST3 SOCCER MANAGER");
    titleLabel->setFont(QFont("Helvetica Neue", 32, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #2dcc73;");
    titleLayout->addWidget(titleLabel);

    auto* subtitleLabel = new QLabel("A Text Simulation Football Manager");
    subtitleLabel->setFont(QFont("Helvetica Neue", 14));
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #6b7c93;");
    titleLayout->addWidget(subtitleLabel);
    titleLayout->addSpacing(40);

    auto* newGameBtn = new QPushButton("New Game");
    newGameBtn->setObjectName("titleBtnPrimary");
    newGameBtn->setFixedSize(220, 44);
    titleLayout->addWidget(newGameBtn, 0, Qt::AlignCenter);

    auto* loadGameBtn = new QPushButton("Load Game");
    loadGameBtn->setObjectName("titleBtn");
    loadGameBtn->setFixedSize(220, 44);
    titleLayout->addWidget(loadGameBtn, 0, Qt::AlignCenter);
    titleLayout->addSpacing(6);

    auto* quitBtn = new QPushButton("Quit");
    quitBtn->setObjectName("titleBtn");
    quitBtn->setFixedSize(220, 44);
    titleLayout->addWidget(quitBtn, 0, Qt::AlignCenter);

    connect(newGameBtn, &QPushButton::clicked, this, &MainWindow::onNewGame);
    connect(loadGameBtn, &QPushButton::clicked, this, &MainWindow::onLoadGame);
    connect(quitBtn, &QPushButton::clicked, qApp, &QApplication::quit);

    outerStack->addWidget(titlePage);

    // ========================================================================
    // TEAM SELECTION SCREEN
    // ========================================================================
    teamSelectPage = new QWidget;
    auto* tsLayout = new QVBoxLayout(teamSelectPage);
    tsLayout->setContentsMargins(40, 30, 40, 30);

    auto* tsTitle = new QLabel("Choose Your Team");
    tsTitle->setFont(QFont("Helvetica Neue", 22, QFont::Bold));
    tsTitle->setAlignment(Qt::AlignCenter);
    tsTitle->setStyleSheet("color: #2dcc73;");
    tsLayout->addWidget(tsTitle);
    tsLayout->addSpacing(10);

    teamList = new QListWidget;
    teamList->setFont(QFont("Courier", 13));
    teamList->setAlternatingRowColors(true);
    tsLayout->addWidget(teamList);

    auto* selectBtn = new QPushButton("Select Team");
    selectBtn->setObjectName("titleBtnPrimary");
    selectBtn->setFixedHeight(40);
    tsLayout->addWidget(selectBtn);
    connect(selectBtn, &QPushButton::clicked, this, [this]() {
        int row = teamList->currentRow();
        if (row >= 0) onTeamSelected(row);
    });
    connect(teamList, &QListWidget::itemDoubleClicked, this, [this]() {
        int row = teamList->currentRow();
        if (row >= 0) onTeamSelected(row);
    });

    outerStack->addWidget(teamSelectPage);

    // ========================================================================
    // GAME PAGE (sidebar + top bar + content)
    // ========================================================================
    gamePage = new QWidget;
    auto* gameLayout = new QVBoxLayout(gamePage);
    gameLayout->setContentsMargins(0, 0, 0, 0);
    gameLayout->setSpacing(0);

    // ---- TOP BAR ----
    auto* topBar = new QWidget;
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(52);
    auto* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(16, 0, 16, 0);

    topBarTeamName = new QLabel;
    topBarTeamName->setFont(QFont("Helvetica Neue", 16, QFont::Bold));
    topBarTeamName->setStyleSheet("color: #2dcc73;");
    topBarLayout->addWidget(topBarTeamName);

    topBarLayout->addStretch();

    topBarInfo = new QLabel;
    topBarInfo->setFont(QFont("Helvetica Neue", 11));
    topBarInfo->setStyleSheet("color: #8b949e;");
    topBarLayout->addWidget(topBarInfo);

    gameLayout->addWidget(topBar);

    // ---- MAIN AREA (sidebar + content) ----
    auto* mainArea = new QWidget;
    auto* mainLayout = new QHBoxLayout(mainArea);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---- SIDEBAR ----
    sidebar = new QWidget;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(170);
    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 8, 0, 8);
    sidebarLayout->setSpacing(0);

    // Sidebar navigation buttons
    struct NavItem { QString text; };
    NavItem navItems[] = {
        {"Home"},
        {"Squad"},
        {"Tactics"},
        {"League Table"},
        {"Fixtures"},
        {"Transfers"},
        {"Top Scorers"},
    };

    for (int i = 0; i < NUM_NAV_BTNS; i++) {
        navBtns[i] = new QPushButton(navItems[i].text);
        navBtns[i]->setObjectName("sidebarBtn");
        navBtns[i]->setCursor(Qt::PointingHandCursor);
        sidebarLayout->addWidget(navBtns[i]);
        int pageIdx = i;
        connect(navBtns[i], &QPushButton::clicked, this, [this, pageIdx]() {
            navigateTo(pageIdx);
        });
    }

    sidebarLayout->addStretch();

    // Play Match — special green accent button
    playMatchBtn = new QPushButton("Play Match");
    playMatchBtn->setObjectName("playMatchBtn");
    playMatchBtn->setCursor(Qt::PointingHandCursor);
    connect(playMatchBtn, &QPushButton::clicked, this, &MainWindow::onPlayMatch);
    sidebarLayout->addWidget(playMatchBtn);
    sidebarLayout->addSpacing(6);

    // Save Game — subtle button at the bottom
    saveGameBtn = new QPushButton("Save Game");
    saveGameBtn->setObjectName("saveBtn");
    saveGameBtn->setCursor(Qt::PointingHandCursor);
    connect(saveGameBtn, &QPushButton::clicked, this, &MainWindow::onSaveGame);
    sidebarLayout->addWidget(saveGameBtn);

    mainLayout->addWidget(sidebar);

    // ---- CONTENT STACK ----
    contentStack = new QStackedWidget;
    mainLayout->addWidget(contentStack);

    gameLayout->addWidget(mainArea);
    outerStack->addWidget(gamePage);

    // ========================================================================
    // DASHBOARD PAGE (Home)
    // ========================================================================
    dashboardPage = new QWidget;
    auto* dashLayout = new QVBoxLayout(dashboardPage);
    dashLayout->setContentsMargins(24, 20, 24, 20);
    dashboardLabel = new QLabel;
    dashboardLabel->setFont(QFont("Helvetica Neue", 12));
    dashboardLabel->setWordWrap(true);
    dashboardLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    dashLayout->addWidget(dashboardLabel);
    dashLayout->addStretch();
    contentStack->addWidget(dashboardPage);
    navPages[0] = dashboardPage;

    // ========================================================================
    // SUB-WIDGETS — created once, added to content stack
    // ========================================================================
    squadWidget = new SquadWidget;
    connect(squadWidget, &SquadWidget::playerSelected, this, &MainWindow::onPlayerSelected);
    contentStack->addWidget(squadWidget);
    navPages[1] = squadWidget;

    tacticsWidget = new TacticsWidget;
    contentStack->addWidget(tacticsWidget);
    navPages[2] = tacticsWidget;

    leagueTableWidget = new LeagueTableWidget;
    contentStack->addWidget(leagueTableWidget);
    navPages[3] = leagueTableWidget;

    fixturesWidget = new FixturesWidget;
    contentStack->addWidget(fixturesWidget);
    navPages[4] = fixturesWidget;

    transferWidget = new TransferWidget;
    contentStack->addWidget(transferWidget);
    navPages[5] = transferWidget;

    topScorersWidget = new TopScorersWidget;
    contentStack->addWidget(topScorersWidget);
    navPages[6] = topScorersWidget;

    // Player profile — accessed via squad double-click, not a sidebar button
    playerProfileWidget = new PlayerProfileWidget;
    connect(playerProfileWidget, &PlayerProfileWidget::backClicked, this, [this]() {
        navigateTo(1); // Back to squad
    });
    contentStack->addWidget(playerProfileWidget);

    // Match widget — accessed via Play Match button, not a sidebar button
    matchWidget = new MatchWidget;
    connect(matchWidget, &MatchWidget::matchFinished, this, &MainWindow::onMatchFinished);
    contentStack->addWidget(matchWidget);

    showTitleScreen();
}

// ============================================================================
// NAVIGATION
// ============================================================================
// Switches the content area to the requested page and highlights the active sidebar button.
// Also refreshes the target widget with current data so it's always up-to-date.
void MainWindow::navigateTo(int pageIndex) {
    // Update sidebar button highlighting
    for (int i = 0; i < NUM_NAV_BTNS; i++) {
        navBtns[i]->setProperty("active", i == pageIndex);
        // Force Qt to re-evaluate the stylesheet for this widget
        navBtns[i]->style()->unpolish(navBtns[i]);
        navBtns[i]->style()->polish(navBtns[i]);
    }

    // Refresh the target widget with current data before showing it
    auto& team = league.teams[playerTeamIdx];
    switch (pageIndex) {
        case 0: updateDashboard(); break;
        case 1: squadWidget->refresh(team); break;
        case 2: tacticsWidget->refresh(team); break;
        case 3: leagueTableWidget->refresh(league); break;
        case 4: fixturesWidget->refresh(league, playerTeamIdx); break;
        case 5: transferWidget->refresh(league, market, playerTeamIdx); break;
        case 6: topScorersWidget->refresh(league); break;
    }

    contentStack->setCurrentWidget(navPages[pageIndex]);
    updateTopBar();
}

// ============================================================================
// TOP BAR UPDATE
// ============================================================================
void MainWindow::updateTopBar() {
    auto& team = league.teams[playerTeamIdx];
    topBarTeamName->setText(QString::fromStdString(team.name).toUpper());

    int leaguePos = 0, pts = 0;
    for (int i = 0; i < (int)league.table.size(); i++) {
        if (league.table[i].teamIdx == playerTeamIdx) {
            leaguePos = i + 1;
            pts = league.table[i].points();
            break;
        }
    }

    int week = league.seasonComplete() ? league.totalWeeks() : league.currentWeek + 1;
    topBarInfo->setText(QString("Season %1  |  Week %2/%3  |  Budget: £%4k  |  %5%6 (%7 pts)")
        .arg(league.season)
        .arg(week)
        .arg(league.totalWeeks())
        .arg(team.budget / 1000)
        .arg(leaguePos)
        .arg(leaguePos == 1 ? "st" : leaguePos == 2 ? "nd" : leaguePos == 3 ? "rd" : "th")
        .arg(pts));

    statusBar()->showMessage(QString("%1  |  Season %2  |  Week %3/%4")
        .arg(QString::fromStdString(team.name))
        .arg(league.season).arg(week).arg(league.totalWeeks()));
}

// ============================================================================
// DASHBOARD (Home page content)
// ============================================================================
void MainWindow::updateDashboard() {
    auto& team = league.teams[playerTeamIdx];

    int leaguePos = 0, pts = 0, won = 0, drawn = 0, lost = 0, gf = 0, ga = 0;
    for (auto& e : league.table) {
        if (e.teamIdx == playerTeamIdx) {
            leaguePos = (&e - &league.table[0]) + 1;
            pts = e.points();
            won = e.won; drawn = e.drawn; lost = e.lost;
            gf = e.goalsFor; ga = e.goalsAgainst;
            break;
        }
    }

    // Next match info
    QString nextMatch = "Season complete";
    if (!league.seasonComplete()) {
        auto& fixtures = league.schedule[league.currentWeek];
        for (auto& f : fixtures) {
            if (f.homeTeamIdx == playerTeamIdx) {
                nextMatch = QString("vs %1 (Home)")
                    .arg(QString::fromStdString(league.teams[f.awayTeamIdx].name));
                break;
            }
            if (f.awayTeamIdx == playerTeamIdx) {
                nextMatch = QString("vs %1 (Away)")
                    .arg(QString::fromStdString(league.teams[f.homeTeamIdx].name));
                break;
            }
        }
    }

    int week = league.seasonComplete() ? league.totalWeeks() : league.currentWeek + 1;

    QString html = QString(
        "<div style='margin-bottom: 20px;'>"
        "<span style='font-size: 24px; font-weight: bold; color: #2dcc73;'>%1</span><br>"
        "<span style='color: #6b7c93;'>Season %2 — Week %3 of %4</span>"
        "</div>"
        "<table cellpadding='6' cellspacing='0' style='font-size: 13px;'>"
        "<tr><td style='color: #6b7c93;'>League Position</td>"
            "<td style='font-weight: bold; color: #ffffff; padding-left: 20px;'>%5 / %6</td></tr>"
        "<tr><td style='color: #6b7c93;'>Points</td>"
            "<td style='font-weight: bold; color: #ffffff; padding-left: 20px;'>%7</td></tr>"
        "<tr><td style='color: #6b7c93;'>Record</td>"
            "<td style='padding-left: 20px;'>"
            "<span style='color: #2dcc73;'>%8W</span>&nbsp;&nbsp;"
            "<span style='color: #c8d6e5;'>%9D</span>&nbsp;&nbsp;"
            "<span style='color: #e05555;'>%10L</span></td></tr>"
        "<tr><td style='color: #6b7c93;'>Goal Difference</td>"
            "<td style='padding-left: 20px;'>%11 (%12 scored, %13 conceded)</td></tr>"
        "<tr><td style='color: #6b7c93;'>Formation</td>"
            "<td style='padding-left: 20px;'>%14</td></tr>"
        "<tr><td style='color: #6b7c93;'>Play Style</td>"
            "<td style='padding-left: 20px;'>%15</td></tr>"
        "<tr><td style='color: #6b7c93;'>Budget</td>"
            "<td style='font-weight: bold; padding-left: 20px;'>£%16k</td></tr>"
        "</table>"
        "<div style='margin-top: 24px;'>"
        "<span style='font-size: 15px; font-weight: bold; color: #c8d6e5;'>Next Match</span><br>"
        "<span style='font-size: 14px; color: #2dcc73;'>%17</span>"
        "</div>")
        .arg(QString::fromStdString(team.name))
        .arg(league.season).arg(week).arg(league.totalWeeks())
        .arg(leaguePos).arg(league.teams.size())
        .arg(pts)
        .arg(won).arg(drawn).arg(lost)
        .arg(gf - ga > 0 ? QString("+%1").arg(gf - ga) : QString::number(gf - ga))
        .arg(gf).arg(ga)
        .arg(QString::fromStdString(team.tactics.formationStr()))
        .arg(QString::fromStdString(team.tactics.playStyleStr()))
        .arg(team.budget / 1000)
        .arg(nextMatch);

    dashboardLabel->setText(html);
}

// ============================================================================
// SIDEBAR ENABLE/DISABLE (used during matches)
// ============================================================================
void MainWindow::setSidebarEnabled(bool enabled) {
    for (int i = 0; i < NUM_NAV_BTNS; i++) {
        navBtns[i]->setEnabled(enabled);
    }
    playMatchBtn->setEnabled(enabled);
    saveGameBtn->setEnabled(enabled);
}

// ============================================================================
// SCREEN SWITCHING
// ============================================================================
void MainWindow::showTitleScreen() {
    outerStack->setCurrentWidget(titlePage);
}

void MainWindow::showTeamSelect() {
    teamList->clear();
    for (int i = 0; i < (int)league.teams.size(); i++) {
        auto& team = league.teams[i];
        QString entry = QString("%1.  %2    OVR: %3   Budget: £%4k")
            .arg(i + 1, 2)
            .arg(QString::fromStdString(team.name), -22)
            .arg(team.averageOverall(), 0, 'f', 0)
            .arg(team.budget / 1000);
        teamList->addItem(entry);
    }
    teamList->setCurrentRow(0);
    outerStack->setCurrentWidget(teamSelectPage);
}

void MainWindow::showGameUI() {
    outerStack->setCurrentWidget(gamePage);
    navigateTo(0);
}

// ============================================================================
// GAME FLOW
// ============================================================================
void MainWindow::onNewGame() {
    league.initialize();
    showTeamSelect();
}

void MainWindow::onLoadGame() {
    if (SaveLoad::loadGame(league, playerTeamIdx, SaveLoad::defaultSavePath())) {
        market.generateListings(league, playerTeamIdx);
        showGameUI();
    } else {
        QMessageBox::information(this, "Load Game", "No save file found.");
    }
}

void MainWindow::onTeamSelected(int index) {
    playerTeamIdx = index;
    market.generateListings(league, playerTeamIdx);

    QMessageBox::information(this, "Welcome!",
        QString("You are now the manager of %1!\n\nBudget: £%2k\nSquad size: %3 players")
            .arg(QString::fromStdString(league.teams[playerTeamIdx].name))
            .arg(league.teams[playerTeamIdx].budget / 1000)
            .arg(league.teams[playerTeamIdx].squad.size()));

    showGameUI();
}

void MainWindow::onPlayMatch() {
    if (league.seasonComplete()) {
        onEndOfSeason();
        return;
    }

    auto& fixtures = league.currentFixtures();

    int playerFixtureIdx = -1;
    for (int i = 0; i < (int)fixtures.size(); i++) {
        if (fixtures[i].homeTeamIdx == playerTeamIdx || fixtures[i].awayTeamIdx == playerTeamIdx) {
            playerFixtureIdx = i;
        }
    }

    // Simulate AI matches
    for (int i = 0; i < (int)fixtures.size(); i++) {
        if (i == playerFixtureIdx) continue;
        auto& f = fixtures[i];
        f.result = engine.simulate(league.teams[f.homeTeamIdx], league.teams[f.awayTeamIdx]);
        f.played = true;
        league.updateTable(f);
    }

    // Start player's match
    if (playerFixtureIdx >= 0) {
        auto& f = fixtures[playerFixtureIdx];
        matchWidget->startMatch(league, engine, f, playerFixtureIdx);
        contentStack->setCurrentWidget(matchWidget);
        setSidebarEnabled(false);
    }

    league.currentWeek++;
}

void MainWindow::onMatchFinished() {
    setSidebarEnabled(true);

    int prevWeek = league.currentWeek - 1;
    if (prevWeek >= 0 && prevWeek < (int)league.schedule.size()) {
        for (auto& f : league.schedule[prevWeek]) {
            if ((f.homeTeamIdx == playerTeamIdx || f.awayTeamIdx == playerTeamIdx) && f.played) {
                league.updateTable(f);
                break;
            }
        }
    }
    navigateTo(0);
}

void MainWindow::onSaveGame() {
    if (SaveLoad::saveGame(league, playerTeamIdx, SaveLoad::defaultSavePath())) {
        QMessageBox::information(this, "Save", "Game saved successfully!");
    } else {
        QMessageBox::warning(this, "Save", "Error saving game.");
    }
}

void MainWindow::onPlayerSelected(int squadIndex) {
    auto& team = league.teams[playerTeamIdx];
    if (squadIndex < 0 || squadIndex >= (int)team.squad.size()) return;

    auto xi = team.getStartingEleven();
    bool isStarter = std::find(xi.begin(), xi.end(), squadIndex) != xi.end();

    playerProfileWidget->refresh(team.squad[squadIndex], team.name, isStarter);
    contentStack->setCurrentWidget(playerProfileWidget);
    // Keep "Squad" highlighted since player profile is a sub-view of squad
}

void MainWindow::onEndOfSeason() {
    QString champion = QString::fromStdString(league.teams[league.table[0].teamIdx].name);
    int playerPos = 0;
    for (int i = 0; i < (int)league.table.size(); i++) {
        if (league.table[i].teamIdx == playerTeamIdx) { playerPos = i + 1; break; }
    }

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
            for (auto& p : team.squad) {
                p.goals = 0; p.assists = 0; p.appearances = 0; p.age++;
            }
            team.squad.erase(
                std::remove_if(team.squad.begin(), team.squad.end(),
                    [](const Player& p) { return p.age > 36; }),
                team.squad.end());

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

        league.table.clear();
        for (int i = 0; i < (int)league.teams.size(); i++) {
            LeagueEntry entry; entry.teamIdx = i; league.table.push_back(entry);
        }
        league.generateSchedule();
        market.refreshListings(league, playerTeamIdx);
        navigateTo(0);
    }
}
