// LeagueTableWidget.cpp
// Implementation of the league table display.
// Builds a standard football league table with color coding:
//   - Gold row for 1st place (champions)
//   - Red rows for the bottom 3 (relegation zone)
//   - Bold text for the Points column

#include "LeagueTableWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>

LeagueTableWidget::LeagueTableWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    headerLabel = new QLabel;
    headerLabel->setFont(QFont("Helvetica", 16, QFont::Bold));
    layout->addWidget(headerLabel);

    // Table configuration — read-only, row selection, not sortable (table is pre-sorted)
    table = new QTableWidget;
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setFont(QFont("Courier", 12));
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);

    auto* backBtn = new QPushButton("Back");
    backBtn->setFixedWidth(100);
    connect(backBtn, &QPushButton::clicked, this, &LeagueTableWidget::backClicked);
    layout->addWidget(backBtn);
}

// ============================================================================
// REFRESH
// ============================================================================
// Rebuilds the league table display from the current League data.
// The league.table vector is already sorted by points/GD/goals, so we just
// iterate through it and assign positions 1, 2, 3, etc.
void LeagueTableWidget::refresh(const League& league) {
    headerLabel->setText(QString("League Table — Season %1").arg(league.season));

    QStringList headers = {"Pos", "Team", "P", "W", "D", "L", "GF", "GA", "GD", "Pts"};
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setRowCount((int)league.table.size());

    for (int i = 0; i < (int)league.table.size(); i++) {
        auto& e = league.table[i];
        auto& team = league.teams[e.teamIdx];
        int col = 0;

        // Helper lambda: creates a centered cell with the correct background color.
        // Row 0 (1st place) gets a gold tint, bottom 3 rows get a red tint.
        auto setItem = [&](const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            // Color coding: gold for champions, red for relegation zone
            if (i == 0) item->setBackground(QColor(255, 215, 0, 60));          // Gold, semi-transparent
            else if (i >= (int)league.table.size() - 3) item->setBackground(QColor(255, 200, 200));  // Light red
            table->setItem(i, col++, item);
        };

        setItem(QString::number(i + 1));  // Position: 1, 2, 3, ...

        // Team name gets the same background color but isn't centered (left-aligned by default)
        auto* nameItem = new QTableWidgetItem(QString::fromStdString(team.name));
        if (i == 0) nameItem->setBackground(QColor(255, 215, 0, 60));
        else if (i >= (int)league.table.size() - 3) nameItem->setBackground(QColor(255, 200, 200));
        table->setItem(i, col++, nameItem);

        // Statistics columns
        setItem(QString::number(e.played));           // P  - Played
        setItem(QString::number(e.won));              // W  - Won
        setItem(QString::number(e.drawn));            // D  - Drawn
        setItem(QString::number(e.lost));             // L  - Lost
        setItem(QString::number(e.goalsFor));         // GF - Goals For
        setItem(QString::number(e.goalsAgainst));     // GA - Goals Against
        setItem(QString::number(e.goalDifference())); // GD - Goal Difference

        // Points column: BOLD because it's the most important number in the table
        auto* ptsItem = new QTableWidgetItem(QString::number(e.points()));
        ptsItem->setTextAlignment(Qt::AlignCenter);
        ptsItem->setFont(QFont("Courier", 12, QFont::Bold));
        if (i == 0) ptsItem->setBackground(QColor(255, 215, 0, 60));
        else if (i >= (int)league.table.size() - 3) ptsItem->setBackground(QColor(255, 200, 200));
        table->setItem(i, col, ptsItem);
    }

    table->resizeColumnsToContents();
}
