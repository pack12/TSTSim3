// SquadWidget.h
// The squad management screen — shows all players on your team in a sortable table.
//
// Features:
//   - 16-column table: #, Name, Pos, Age, OVR, PAC, PAS, SHO, DEF, DRI, GK, STA, CRE, Gls, Ast, App
//   - All columns are sortable (click the column header to sort by that attribute)
//   - Starting XI players are highlighted in green
//   - Double-click a player to view their full profile (emits playerSelected signal)
//   - Shows total squad value at the bottom
//
// The table uses Qt's DisplayRole with integer values (not strings) so that sorting
// works numerically. For example, if you sort by OVR, a 15-rated player appears above
// a 9-rated player (string sorting would put "9" after "15" because "9" > "1").
//
// The original squad index is stored in Qt::UserRole on the first column (#) so that
// even after the user sorts the table, we can still look up the correct player in the
// squad vector. Without this, double-clicking row 3 after sorting by OVR would give
// us the wrong player.

#pragma once
#include "Team.h"
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>

class SquadWidget : public QWidget {
    Q_OBJECT
public:
    explicit SquadWidget(QWidget* parent = nullptr);

    // Populates the table with the given team's squad data.
    // teamIdx: index into League::teams[] for this team
    // playerTeamIdx: the user's own team (used to show/hide "Back" and enable editing)
    void refresh(const Team& team, int teamIdx, int playerTeamIdx);

    // Pointer to the team currently being displayed.
    const Team* currentTeam = nullptr;

signals:
    void playerSelected(int squadIndex);    // Emitted when a player row is double-clicked
    void backRequested();                   // Emitted when "Back" is clicked (viewing another team)

private:
    QTableWidget* table = nullptr;      // The main sortable table
    QLabel* headerLabel = nullptr;      // "Alderwick City — Squad" at the top
    QLabel* valueLabel = nullptr;       // Squad value and instructions at the bottom
    QPushButton* backBtn = nullptr;     // "Back" button — visible when viewing another team's squad

    int currentTeamIdx = -1;            // Which team is being displayed
    int currentPlayerTeamIdx = -1;      // The user's own team index
};
