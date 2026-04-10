// LeagueTableWidget.h
// The league table screen — shows the standings of all 16 teams.
//
// Features:
//   - Standard football league table columns: Pos, Team, P, W, D, L, GF, GA, GD, Pts
//   - First place is highlighted in gold (the champion's row)
//   - Bottom 3 teams are highlighted in red (relegation zone)
//   - Points column is in bold (it's the most important number)
//
// This widget does NOT use sortable columns — the league table is always displayed
// in the correct sorted order (by points, then goal difference, then goals scored),
// because League::sortTable() keeps it sorted at all times.

#pragma once
#include "League.h"
#include <QWidget>
#include <QTableWidget>
#include <QLabel>

class LeagueTableWidget : public QWidget {
    Q_OBJECT
public:
    explicit LeagueTableWidget(QWidget* parent = nullptr);
    void refresh(const League& league);

private:
    QTableWidget* table = nullptr;
    QLabel* headerLabel = nullptr;  // Shows "League Table — Season X"
};
