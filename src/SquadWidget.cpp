// SquadWidget.cpp
// Implementation of the squad table screen.
//
// KEY DESIGN DECISIONS:
//
// 1. NUMERIC SORTING: All numeric columns use setData(Qt::DisplayRole, intValue) instead
//    of setText("15"). This tells Qt "this cell contains the integer 15" so sorting works
//    numerically (1, 2, 10, 15) instead of alphabetically ("1", "10", "15", "2").
//
// 2. ORIGINAL INDEX TRACKING: The first column (#) stores the original squad index in
//    Qt::UserRole (a hidden data slot). When the user double-clicks a row to view a player,
//    we read this hidden value to find the right player in squad[], even if the table has
//    been sorted and the rows are in a different order.
//
// 3. SORTING TOGGLE: We disable sorting before populating the table (setSortingEnabled(false))
//    and re-enable it after. If sorting is enabled during population, Qt re-sorts the table
//    after EVERY row insertion, which is extremely slow and can mix up which data goes in which row.

#include "SquadWidget.h"
#include "LinkUtils.h"
#include <QVBoxLayout>
#include <QHeaderView>

SquadWidget::SquadWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    // Back button — only visible when viewing another team's squad
    backBtn = new QPushButton("← Back");
    backBtn->setFixedWidth(100);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setVisible(false);
    connect(backBtn, &QPushButton::clicked, this, &SquadWidget::backRequested);
    layout->addWidget(backBtn);

    // Header label showing the team name
    headerLabel = new QLabel;
    headerLabel->setFont(QFont("Helvetica", 16, QFont::Bold));
    layout->addWidget(headerLabel);

    // The main table — configured for read-only, row selection, alternating colors
    table = new QTableWidget;
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);    // Can't edit cells
    table->setSelectionBehavior(QAbstractItemView::SelectRows);   // Clicking selects the whole row
    table->setAlternatingRowColors(true);                          // Zebra striping
    table->setSortingEnabled(true);                                // Column headers are clickable for sorting
    table->horizontalHeader()->setStretchLastSection(true);        // Last column fills remaining space
    table->horizontalHeader()->setSortIndicatorShown(true);        // Show the little arrow on sorted column
    table->setFont(QFont("Courier", 12));                          // Monospace font for alignment
    layout->addWidget(table);

    // Hover-reveal: player name links light up green on mouse-over
    LinkUtils::installHoverHandler(table);

    // Footer label showing squad value and instructions
    valueLabel = new QLabel;
    valueLabel->setFont(QFont("Helvetica", 12));
    layout->addWidget(valueLabel);

    // Single-click on a link cell navigates to that player's profile
    connect(table, &QTableWidget::cellClicked, this, [this](int row, int col) {
        auto* item = table->item(row, col);
        if (!item) return;
        QVariant pv = item->data(LinkUtils::PlayerIdxRole);
        if (pv.isValid()) {
            emit playerSelected(pv.toInt());
        }
    });

    // Double-click handler: read the original squad index from UserRole and emit it
    connect(table, &QTableWidget::cellDoubleClicked, this, [this](int row, int /*col*/) {
        auto* item = table->item(row, 0);  // First column has the UserRole data
        if (item) {
            int squadIdx = item->data(Qt::UserRole).toInt();
            emit playerSelected(squadIdx);
        }
    });
}

// ============================================================================
// REFRESH
// ============================================================================
// Rebuilds the entire table with the given team's data.
// Called every time the player navigates to this screen.
void SquadWidget::refresh(const Team& team, int teamIdx, int playerTeamIdx) {
    currentTeam = &team;
    currentTeamIdx = teamIdx;
    currentPlayerTeamIdx = playerTeamIdx;

    bool isOwnTeam = (teamIdx == playerTeamIdx);
    backBtn->setVisible(!isOwnTeam);

    QString title = QString::fromStdString(team.name) + " — Squad";
    if (!isOwnTeam) title += "  (Scouting)";
    headerLabel->setText(title);

    // CRITICAL: Disable sorting while populating! See explanation in the file header.
    table->setSortingEnabled(false);

    // Set up column headers
    QStringList headers = {"#", "Name", "Pos", "Age", "OVR",
        "PAC", "PAS", "SHO", "DEF", "DRI", "GK", "STA", "CRE",
        "Gls", "Ast", "App"};
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setRowCount((int)team.squad.size());

    // Get the starting XI so we can highlight those players in green
    auto xi = team.getStartingEleven();

    for (int i = 0; i < (int)team.squad.size(); i++) {
        auto& p = team.squad[i];
        bool isStarter = std::find(xi.begin(), xi.end(), i) != xi.end();
        // Starters get a dark green tint; subs get the base dark background
        QColor bg = isStarter ? QColor(25, 60, 35) : QColor(14, 22, 33);
        int col = 0;

        // Helper lambda: creates a numeric cell with centered text and correct background
        auto setNumItem = [&](int value) {
            auto* item = new QTableWidgetItem;
            item->setData(Qt::DisplayRole, value);   // Store as int for numeric sorting
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(bg);
            table->setItem(i, col++, item);
        };

        // Helper lambda: creates a text cell with optional centering and correct background
        auto setTextItem = [&](const QString& text, bool center = true) {
            auto* item = new QTableWidgetItem(text);
            if (center) item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(bg);
            table->setItem(i, col++, item);
        };

        // First column (#): shows the player number, and ALSO stores the original squad index
        // in Qt::UserRole. This hidden data is how we find the right player after sorting.
        auto* idxItem = new QTableWidgetItem;
        idxItem->setData(Qt::DisplayRole, i + 1);    // Display: 1-based number
        idxItem->setData(Qt::UserRole, i);            // Hidden: 0-based squad index
        idxItem->setTextAlignment(Qt::AlignCenter);
        idxItem->setBackground(bg);
        table->setItem(i, col++, idxItem);

        // Player name — hover-reveal link to player profile
        auto* nameItem = LinkUtils::makePlayerLink(QString::fromStdString(p.name), currentTeamIdx, i);
        nameItem->setBackground(bg);
        table->setItem(i, col++, nameItem);
        setTextItem(QString::fromStdString(p.positionStr()));
        setNumItem(p.age);
        setNumItem(p.overall());

        // All 8 attributes
        setNumItem(p.pace);
        setNumItem(p.passing);
        setNumItem(p.shooting);
        setNumItem(p.defending);
        setNumItem(p.dribbling);
        setNumItem(p.goalkeeping);
        setNumItem(p.stamina);
        setNumItem(p.creativity);

        // Season stats
        setNumItem(p.goals);
        setNumItem(p.assists);
        setNumItem(p.appearances);
    }

    // Re-enable sorting now that all data is in place
    table->setSortingEnabled(true);
    table->resizeColumnsToContents();  // Auto-fit column widths

    // Calculate and display total squad value
    int total = 0;
    for (auto& p : team.squad) total += p.marketValue();
    // Format as £X.YM for millions or £Xk for thousands
    QString valStr = total >= 1000000
        ? QString("£%1.%2M").arg(total / 1000000).arg((total % 1000000) / 100000)
        : QString("£%1k").arg(total / 1000);

    QString footer = "Squad Value: " + valStr + "  |  Starting XI highlighted in green";
    footer += isOwnTeam ? "  |  Double-click a player for profile"
                        : "  |  Double-click to scout a player";
    valueLabel->setText(footer);
}
