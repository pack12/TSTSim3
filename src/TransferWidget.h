// TransferWidget.h
// The transfer market screen — split into two panels:
//   Left (60%): Available players from AI teams that you can buy
//   Right (40%): Your own squad with the option to sell players
//
// Features:
//   - Both tables are sortable (click column headers)
//   - "Buy Selected" button to purchase a player from the market
//   - "Sell Selected" button to sell one of your own players
//   - "Refresh Listings" button to regenerate the available players
//   - Confirmation dialogs before every transaction
//   - Budget display at the top that updates after each transaction
//
// Like SquadWidget, original data indices are stored in Qt::UserRole on the first column
// so that we can find the correct player/listing even after the user sorts the table.

#pragma once
#include "League.h"
#include "TransferMarket.h"
#include <QWidget>
#include <QTableWidget>
#include <QLabel>

class TransferWidget : public QWidget {
    Q_OBJECT
public:
    explicit TransferWidget(QWidget* parent = nullptr);
    void refresh(League& league, TransferMarket& market, int playerTeamIdx);

signals:
    void backClicked();

private slots:
    void onBuy();               // Buy the selected player from the market
    void onSell();              // Sell the selected player from your squad
    void onRefreshListings();   // Regenerate available players

private:
    QTableWidget* marketTable = nullptr;    // Left panel: available players
    QTableWidget* squadTable = nullptr;     // Right panel: your squad
    QLabel* budgetLabel = nullptr;          // "Your Budget: £Xk" at the top
    QLabel* headerLabel = nullptr;          // "Transfer Market" title

    // Pointers to the game data — needed by buy/sell handlers
    League* currentLeague = nullptr;
    TransferMarket* currentMarket = nullptr;
    int currentPlayerTeamIdx = 0;
};
