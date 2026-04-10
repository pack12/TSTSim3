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
    void refresh(const Player& player, const std::string& teamName, bool isStarter);

signals:
    void backClicked();  // Navigates back to Squad view

private:
    // Builds one attribute bar: a label, a QProgressBar, and a numeric value.
    // Returns a QWidget containing the whole row (label + bar + number).
    // The bar's color changes based on the value/maxVal ratio.
    QWidget* buildAttrBar(const QString& label, int value, int maxVal = 20);

    QLabel* nameLabel = nullptr;            // Player name at the top
    QLabel* infoLabel = nullptr;            // Position, age, overall, club, value, starter status
    QWidget* attrsContainer = nullptr;      // Container for the attribute bar grid
    QLabel* statsLabel = nullptr;           // Season statistics table
};
