// TopScorersWidget.cpp
// Implementation of the top scorers table.
// Aggregates scoring data from ALL players across ALL teams in the league,
// filters to those with at least 1 goal or assist, sorts by goals (then assists),
// and displays the top 30.

#include "TopScorersWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <algorithm>

TopScorersWidget::TopScorersWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    auto* title = new QLabel("Top Scorers");
    title->setFont(QFont("Helvetica", 16, QFont::Bold));
    layout->addWidget(title);

    table = new QTableWidget;
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setFont(QFont("Courier", 12));
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);
}

// ============================================================================
// REFRESH
// ============================================================================
// Rebuilds the top scorers table from the current league data.
//
// Steps:
//   1. Loop through every player on every team
//   2. Collect anyone with goals > 0 or assists > 0 into a flat list
//   3. Sort by goals (descending), then assists (descending) as tiebreaker
//   4. Display the top 30 (or fewer if there aren't 30 scorers)
//   5. Bold the top 3 entries (Golden Boot contenders)
void TopScorersWidget::refresh(const League& league) {
    // Temporary struct to hold scorer data for sorting
    struct Scorer { std::string name; std::string team; int goals; int assists; };
    std::vector<Scorer> scorers;

    // Collect all players with goals or assists from every team
    for (auto& team : league.teams) {
        for (auto& p : team.squad) {
            if (p.goals > 0 || p.assists > 0) {
                scorers.push_back({p.name, team.name, p.goals, p.assists});
            }
        }
    }

    // Sort: most goals first. If goals are tied, most assists wins.
    std::sort(scorers.begin(), scorers.end(), [](auto& a, auto& b) {
        return a.goals != b.goals ? a.goals > b.goals : a.assists > b.assists;
    });

    QStringList headers = {"#", "Player", "Team", "Goals", "Assists"};
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);

    int count = std::min((int)scorers.size(), 30);  // Cap at 30 entries
    table->setRowCount(count);

    for (int i = 0; i < count; i++) {
        int col = 0;

        // Helper lambda: creates a cell, optionally centered, bold if in top 3
        auto setItem = [&](const QString& text, bool center = true) {
            auto* item = new QTableWidgetItem(text);
            if (center) item->setTextAlignment(Qt::AlignCenter);
            if (i < 3) item->setFont(QFont("Courier", 12, QFont::Bold));  // Top 3 in bold
            table->setItem(i, col++, item);
        };

        setItem(QString::number(i + 1));                              // Rank: 1, 2, 3, ...
        setItem(QString::fromStdString(scorers[i].name), false);      // Player name (left-aligned)
        setItem(QString::fromStdString(scorers[i].team), false);      // Team name (left-aligned)
        setItem(QString::number(scorers[i].goals));                    // Goals
        setItem(QString::number(scorers[i].assists));                  // Assists
    }

    table->resizeColumnsToContents();
}
