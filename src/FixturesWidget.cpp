// FixturesWidget.cpp
// Implementation of the fixtures/results screen.
// Shows 8 matches per week (16 teams / 2 = 8 matches per round).
// Uses a QSpinBox to browse between weeks 1-30.

#include "FixturesWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>

FixturesWidget::FixturesWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    headerLabel = new QLabel("Fixtures & Results");
    headerLabel->setFont(QFont("Helvetica", 16, QFont::Bold));
    layout->addWidget(headerLabel);

    // Week selector row: "Week: [spinner]"
    auto* weekRow = new QHBoxLayout;
    weekRow->addWidget(new QLabel("Week:"));
    weekSpinner = new QSpinBox;
    weekSpinner->setMinimum(1);
    weekSpinner->setFont(QFont("Helvetica", 13));
    weekRow->addWidget(weekSpinner);
    weekRow->addStretch();  // Push the spinner to the left
    layout->addLayout(weekRow);

    // Fixtures table
    table = new QTableWidget;
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setFont(QFont("Courier", 12));
    table->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table);

    auto* backBtn = new QPushButton("Back");
    backBtn->setFixedWidth(100);
    connect(backBtn, &QPushButton::clicked, this, &FixturesWidget::backClicked);
    layout->addWidget(backBtn);

    // Connect spinner changes — uses QOverload because QSpinBox::valueChanged has two overloads
    connect(weekSpinner, QOverload<int>::of(&QSpinBox::valueChanged), this, &FixturesWidget::onWeekChanged);
}

// ============================================================================
// REFRESH
// ============================================================================
// Sets up the spinner range and navigates to the current (or most recent) week.
// Uses blockSignals to prevent the spinner from triggering onWeekChanged while
// we're programmatically setting its value.
void FixturesWidget::refresh(const League& league, int playerTeamIdx) {
    currentLeague = &league;
    currentPlayerTeamIdx = playerTeamIdx;

    weekSpinner->blockSignals(true);
    weekSpinner->setMaximum(league.totalWeeks());  // Usually 30
    // Default to current week (or last week if season is complete)
    int displayWeek = league.seasonComplete() ? league.totalWeeks() : league.currentWeek + 1;
    weekSpinner->setValue(displayWeek);
    weekSpinner->blockSignals(false);

    onWeekChanged(displayWeek);  // Display the fixtures for the selected week
}

// ============================================================================
// ON WEEK CHANGED
// ============================================================================
// Called when the week spinner value changes. Rebuilds the table for the selected week.
// Shows either fixtures (upcoming: "Home vs Away") or results (played: "Home 2 - 1 Away").
void FixturesWidget::onWeekChanged(int week) {
    if (!currentLeague) return;
    int idx = week - 1;  // Convert 1-based display week to 0-based schedule index
    if (idx < 0 || idx >= (int)currentLeague->schedule.size()) return;

    auto& fixtures = currentLeague->schedule[idx];
    // Check if this week's matches have been played (check the first fixture)
    bool hasResults = fixtures.size() > 0 && fixtures[0].played;

    // Column headers change based on whether results are available
    QStringList headers = hasResults
        ? QStringList{"Home", "Score", "Away"}
        : QStringList{"Home", "vs", "Away"};

    table->setColumnCount(3);
    table->setHorizontalHeaderLabels(headers);
    table->setRowCount((int)fixtures.size());

    for (int i = 0; i < (int)fixtures.size(); i++) {
        auto& f = fixtures[i];
        auto& homeName = currentLeague->teams[f.homeTeamIdx].name;
        auto& awayName = currentLeague->teams[f.awayTeamIdx].name;

        // Check if the player's team is involved in this match
        bool isPlayerMatch = (f.homeTeamIdx == currentPlayerTeamIdx || f.awayTeamIdx == currentPlayerTeamIdx);

        auto* homeItem = new QTableWidgetItem(QString::fromStdString(homeName));
        auto* awayItem = new QTableWidgetItem(QString::fromStdString(awayName));

        // Middle column: score if played, "vs" if upcoming
        QString middle;
        if (f.played) {
            middle = QString("%1 - %2").arg(f.result.homeGoals).arg(f.result.awayGoals);
        } else {
            middle = "vs";
        }
        auto* midItem = new QTableWidgetItem(middle);
        midItem->setTextAlignment(Qt::AlignCenter);

        // Highlight the player's match in blue with bold text
        if (isPlayerMatch) {
            QColor highlight(200, 220, 255);  // Light blue
            homeItem->setBackground(highlight);
            midItem->setBackground(highlight);
            awayItem->setBackground(highlight);
            homeItem->setFont(QFont("Courier", 12, QFont::Bold));
            awayItem->setFont(QFont("Courier", 12, QFont::Bold));
        }

        table->setItem(i, 0, homeItem);
        table->setItem(i, 1, midItem);
        table->setItem(i, 2, awayItem);
    }

    table->resizeColumnsToContents();
    table->horizontalHeader()->setStretchLastSection(true);
}
