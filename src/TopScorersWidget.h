// TopScorersWidget.h
// The top scorers screen — shows the top 30 goal scorers across the entire league.
//
// Collects every player with at least 1 goal or assist from all 16 teams,
// sorts them by goals (then assists as a tiebreaker), and displays the top 30.
// The top 3 scorers are shown in bold (the Golden Boot contenders).

#pragma once
#include "League.h"
#include <QWidget>
#include <QTableWidget>

class TopScorersWidget : public QWidget {
    Q_OBJECT
public:
    explicit TopScorersWidget(QWidget* parent = nullptr);
    void refresh(const League& league);

signals:
    void backClicked();

private:
    QTableWidget* table = nullptr;
};
