// PlayerProfileWidget.h
// The player profile screen — shows detailed information about a single player.
// Accessed by double-clicking a player in the squad list.
//
// Displays:
//   - Player name (big and bold at the top)
//   - Position (color-coded: GK=gold, DEF=blue, MID=green, FWD=red)
//   - Age, overall rating, club, market value, starter/substitute status
//   - Eight attribute bars (QProgressBar widgets) with color-coded values:
//       Green (80%+) = excellent, Yellow (40-60%) = average, Red (<25%) = terrible
//   - Season statistics: appearances, goals, assists, goals per game
//
// The attribute bars are arranged in a 2-column grid and rebuilt from scratch
// every time refresh() is called (because the number of attributes is fixed but
// the values change between players).

#pragma once
#include "Player.h"
#include <QWidget>
#include <QLabel>

class PlayerProfileWidget : public QWidget {
    Q_OBJECT
public:
    explicit PlayerProfileWidget(QWidget* parent = nullptr);
    void refresh(const Player& player, const std::string& teamName, bool isStarter, int teamIdx = -1);

signals:
    void backClicked();             // Navigates back to Squad view
    void teamClicked(int teamIdx);  // Emitted when the club name link is clicked

private:
    QWidget* buildAttrBar(const QString& label, int value, int maxVal = 20);

    QLabel* nameLabel = nullptr;
    QLabel* infoLabel = nullptr;
    QWidget* attrsContainer = nullptr;
    QLabel* statsLabel = nullptr;

    int currentTeamIdx = -1;        // Team index for the club name link
};
