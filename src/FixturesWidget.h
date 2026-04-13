// FixturesWidget.h
// The fixtures and results screen — shows the schedule for any week of the season.
//
// Features:
//   - QSpinBox week selector (1 to 30) to browse different weeks
//   - If the week has been played: shows scores (e.g. "Alderwick City  2 - 1  Glendale Rovers")
//   - If the week hasn't been played yet: shows "vs" instead of a score
//   - Your team's matches are highlighted in blue with bold text
//
// On load, it defaults to showing the current week (or the last week if the season is complete).

#pragma once
#include "League.h"
#include <QWidget>
#include <QTableWidget>
#include <QSpinBox>
#include <QLabel>

class FixturesWidget : public QWidget {
    Q_OBJECT
public:
    explicit FixturesWidget(QWidget* parent = nullptr);
    void refresh(const League& league, int playerTeamIdx);

signals:
    void teamClicked(int teamIdx);

private slots:
    void onWeekChanged(int week);

private:
    QTableWidget* table = nullptr;      // The fixtures/results table
    QSpinBox* weekSpinner = nullptr;    // Week selector: 1 to totalWeeks
    QLabel* headerLabel = nullptr;      // "Fixtures & Results" title

    const League* currentLeague = nullptr;  // Pointer to the league data
    int currentPlayerTeamIdx = 0;           // Which team is the player's (for highlighting)
};
