// TransferWidget.cpp
// Implementation of the transfer market screen.
//
// LAYOUT: Uses a QSplitter to divide the screen horizontally:
//   - Left panel (60%): "Available Players" — market listings from AI teams
//   - Right panel (40%): "Your Squad" — your players with sell option
//
// SORTING: Both tables use the same tricks as SquadWidget:
//   - Numeric values stored via Qt::DisplayRole (int, not string) for correct sorting
//   - Original indices stored in Qt::UserRole on the first column
//   - Sorting disabled during population, re-enabled after
//
// PRICES: Market table shows prices in thousands (e.g. "5000" means £5,000k = £5M).
//         Squad table shows market values in thousands too.

#include "TransferWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QSplitter>
#include <QGroupBox>

TransferWidget::TransferWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    headerLabel = new QLabel("Transfer Market");
    headerLabel->setFont(QFont("Helvetica", 16, QFont::Bold));
    layout->addWidget(headerLabel);

    budgetLabel = new QLabel;
    budgetLabel->setFont(QFont("Helvetica", 13));
    layout->addWidget(budgetLabel);

    // QSplitter divides the screen into resizable left and right panels
    auto* splitter = new QSplitter(Qt::Horizontal);

    // ---- LEFT PANEL: MARKET LISTINGS ----
    auto* marketGroup = new QGroupBox("Available Players");
    auto* marketLayout = new QVBoxLayout(marketGroup);
    marketTable = new QTableWidget;
    marketTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    marketTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    marketTable->setAlternatingRowColors(true);
    marketTable->setSortingEnabled(true);
    marketTable->horizontalHeader()->setSortIndicatorShown(true);
    marketTable->setFont(QFont("Courier", 11));
    marketLayout->addWidget(marketTable);

    // Buy and Refresh buttons side by side
    auto* buyBtnLayout = new QHBoxLayout;
    auto* buyBtn = new QPushButton("Buy Selected");
    buyBtn->setFixedHeight(32);
    connect(buyBtn, &QPushButton::clicked, this, &TransferWidget::onBuy);
    buyBtnLayout->addWidget(buyBtn);
    auto* refreshBtn = new QPushButton("Refresh Listings");
    refreshBtn->setFixedHeight(32);
    connect(refreshBtn, &QPushButton::clicked, this, &TransferWidget::onRefreshListings);
    buyBtnLayout->addWidget(refreshBtn);
    marketLayout->addLayout(buyBtnLayout);

    splitter->addWidget(marketGroup);

    // ---- RIGHT PANEL: YOUR SQUAD ----
    auto* squadGroup = new QGroupBox("Your Squad");
    auto* squadLayout = new QVBoxLayout(squadGroup);
    squadTable = new QTableWidget;
    squadTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    squadTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    squadTable->setAlternatingRowColors(true);
    squadTable->setSortingEnabled(true);
    squadTable->horizontalHeader()->setSortIndicatorShown(true);
    squadTable->setFont(QFont("Courier", 11));
    squadLayout->addWidget(squadTable);

    auto* sellBtn = new QPushButton("Sell Selected");
    sellBtn->setFixedHeight(32);
    connect(sellBtn, &QPushButton::clicked, this, &TransferWidget::onSell);
    squadLayout->addWidget(sellBtn);

    splitter->addWidget(squadGroup);
    splitter->setStretchFactor(0, 3);  // Left panel gets 60% of the width
    splitter->setStretchFactor(1, 2);  // Right panel gets 40%

    layout->addWidget(splitter);

    auto* backBtn = new QPushButton("Back");
    backBtn->setFixedWidth(100);
    connect(backBtn, &QPushButton::clicked, this, &TransferWidget::backClicked);
    layout->addWidget(backBtn);
}

// ============================================================================
// REFRESH
// ============================================================================
// Rebuilds both tables from the current data.
// Called initially and after every buy/sell/refresh operation.
void TransferWidget::refresh(League& league, TransferMarket& market, int playerTeamIdx) {
    currentLeague = &league;
    currentMarket = &market;
    currentPlayerTeamIdx = playerTeamIdx;

    budgetLabel->setText(QString("Your Budget: £%1k").arg(league.teams[playerTeamIdx].budget / 1000));

    // ---- POPULATE MARKET TABLE ----
    marketTable->setSortingEnabled(false);  // Disable sorting during population
    QStringList mHeaders = {"Player", "Pos", "Age", "OVR", "Club", "Price"};
    marketTable->setColumnCount(mHeaders.size());
    marketTable->setHorizontalHeaderLabels(mHeaders);
    marketTable->setRowCount((int)market.listings.size());

    for (int i = 0; i < (int)market.listings.size(); i++) {
        auto& l = market.listings[i];
        auto& p = league.teams[l.teamIdx].squad[l.playerIdx];
        int col = 0;

        // First column stores the listing index in UserRole (survives sorting)
        auto* nameItem = new QTableWidgetItem(QString::fromStdString(p.name));
        nameItem->setData(Qt::UserRole, i);
        marketTable->setItem(i, col++, nameItem);

        auto* posItem = new QTableWidgetItem(QString::fromStdString(p.positionStr()));
        posItem->setTextAlignment(Qt::AlignCenter);
        marketTable->setItem(i, col++, posItem);

        // Age and OVR as integers for numeric sorting
        auto* ageItem = new QTableWidgetItem;
        ageItem->setData(Qt::DisplayRole, p.age);
        ageItem->setTextAlignment(Qt::AlignCenter);
        marketTable->setItem(i, col++, ageItem);

        auto* ovrItem = new QTableWidgetItem;
        ovrItem->setData(Qt::DisplayRole, p.overall());
        ovrItem->setTextAlignment(Qt::AlignCenter);
        marketTable->setItem(i, col++, ovrItem);

        marketTable->setItem(i, col++, new QTableWidgetItem(QString::fromStdString(league.teams[l.teamIdx].name)));

        // Price in thousands for numeric sorting
        auto* priceItem = new QTableWidgetItem;
        priceItem->setData(Qt::DisplayRole, l.askingPrice / 1000);
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        marketTable->setItem(i, col++, priceItem);
    }
    marketTable->setSortingEnabled(true);
    marketTable->resizeColumnsToContents();

    // ---- POPULATE SQUAD TABLE ----
    squadTable->setSortingEnabled(false);
    auto& team = league.teams[playerTeamIdx];
    QStringList sHeaders = {"Name", "Pos", "Age", "OVR", "Value"};
    squadTable->setColumnCount(sHeaders.size());
    squadTable->setHorizontalHeaderLabels(sHeaders);
    squadTable->setRowCount((int)team.squad.size());

    for (int i = 0; i < (int)team.squad.size(); i++) {
        auto& p = team.squad[i];
        int col = 0;

        // First column stores the squad index in UserRole
        auto* nameItem = new QTableWidgetItem(QString::fromStdString(p.name));
        nameItem->setData(Qt::UserRole, i);
        squadTable->setItem(i, col++, nameItem);

        auto* posItem = new QTableWidgetItem(QString::fromStdString(p.positionStr()));
        posItem->setTextAlignment(Qt::AlignCenter);
        squadTable->setItem(i, col++, posItem);

        auto* ageItem = new QTableWidgetItem;
        ageItem->setData(Qt::DisplayRole, p.age);
        ageItem->setTextAlignment(Qt::AlignCenter);
        squadTable->setItem(i, col++, ageItem);

        auto* ovrItem = new QTableWidgetItem;
        ovrItem->setData(Qt::DisplayRole, p.overall());
        ovrItem->setTextAlignment(Qt::AlignCenter);
        squadTable->setItem(i, col++, ovrItem);

        auto* valItem = new QTableWidgetItem;
        valItem->setData(Qt::DisplayRole, p.marketValue() / 1000);
        valItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        squadTable->setItem(i, col++, valItem);
    }
    squadTable->setSortingEnabled(true);
    squadTable->resizeColumnsToContents();
}

// ============================================================================
// BUY PLAYER
// ============================================================================
// Handles the "Buy Selected" button click.
// Reads the original listing index from UserRole, shows a confirmation dialog,
// and attempts the purchase. Refreshes the display on success.
void TransferWidget::onBuy() {
    if (!currentMarket || !currentLeague) return;
    int row = marketTable->currentRow();
    if (row < 0) { QMessageBox::information(this, "Buy", "Select a player first."); return; }

    // Read the original listing index from the hidden UserRole data
    int listingIdx = marketTable->item(row, 0)->data(Qt::UserRole).toInt();
    auto& l = currentMarket->listings[listingIdx];
    auto& p = currentLeague->teams[l.teamIdx].squad[l.playerIdx];

    // Confirmation dialog: "Buy James Smith (FWD, OVR 15) for £2500k?"
    auto reply = QMessageBox::question(this, "Confirm Transfer",
        QString("Buy %1 (%2, OVR %3) for £%4k?")
            .arg(QString::fromStdString(p.name))
            .arg(QString::fromStdString(p.positionStr()))
            .arg(p.overall())
            .arg(l.askingPrice / 1000));

    if (reply == QMessageBox::Yes) {
        if (currentMarket->buyPlayer(*currentLeague, currentPlayerTeamIdx, listingIdx)) {
            QMessageBox::information(this, "Transfer Complete", "Player signed!");
            refresh(*currentLeague, *currentMarket, currentPlayerTeamIdx);
        } else {
            QMessageBox::warning(this, "Transfer Failed", "Insufficient budget or seller has too few players.");
        }
    }
}

// ============================================================================
// SELL PLAYER
// ============================================================================
// Handles the "Sell Selected" button click.
// Same pattern: read original squad index from UserRole, confirm, execute, refresh.
void TransferWidget::onSell() {
    if (!currentMarket || !currentLeague) return;
    int row = squadTable->currentRow();
    if (row < 0) { QMessageBox::information(this, "Sell", "Select a player first."); return; }
    // Read the original squad index from UserRole
    int squadIdx = squadTable->item(row, 0)->data(Qt::UserRole).toInt();

    auto& p = currentLeague->teams[currentPlayerTeamIdx].squad[squadIdx];

    auto reply = QMessageBox::question(this, "Confirm Sale",
        QString("Sell %1 (%2, OVR %3, value £%4k)?")
            .arg(QString::fromStdString(p.name))
            .arg(QString::fromStdString(p.positionStr()))
            .arg(p.overall())
            .arg(p.marketValue() / 1000));

    if (reply == QMessageBox::Yes) {
        if (currentMarket->sellPlayer(*currentLeague, currentPlayerTeamIdx, squadIdx)) {
            QMessageBox::information(this, "Player Sold", "Transfer complete.");
            refresh(*currentLeague, *currentMarket, currentPlayerTeamIdx);
        } else {
            QMessageBox::warning(this, "Cannot Sell", "Minimum squad size reached.");
        }
    }
}

// Regenerates the market listings and refreshes the display.
void TransferWidget::onRefreshListings() {
    if (currentMarket && currentLeague) {
        currentMarket->refreshListings(*currentLeague, currentPlayerTeamIdx);
        refresh(*currentLeague, *currentMarket, currentPlayerTeamIdx);
    }
}
