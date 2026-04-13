// LinkUtils.h
// Football Manager-style hover-reveal links for QTableWidgets.
//
// HOW IT WORKS:
// Link cells look like normal text by default. When you hover over a cell that
// has link data (TeamIdxRole / PlayerIdxRole), it lights up in accent green with
// an underline and the cursor changes to a pointing hand — just like FM.
// Move the mouse away and it reverts to normal.
//
// USAGE:
//   1. Create items with makeTeamLink() / makePlayerLink() — they look normal
//   2. Call installHoverHandler(table) once in the widget constructor
//   3. Connect table->cellClicked to check for link data and emit navigation signals

#pragma once
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QEvent>
#include <QMouseEvent>
#include <QColor>
#include <QFont>
#include <QCursor>
#include <QObject>

namespace LinkUtils {

// Custom data roles for entity references. Offset from Qt::UserRole to avoid collisions.
constexpr int TeamIdxRole   = Qt::UserRole + 10;
constexpr int PlayerIdxRole = Qt::UserRole + 11;

// Returns true if a table item has link data attached.
inline bool isLink(QTableWidgetItem* item) {
    return item && item->data(TeamIdxRole).isValid();
}

// Creates a normal-looking item that has team link data attached.
inline QTableWidgetItem* makeTeamLink(const QString& teamName, int teamIdx) {
    auto* item = new QTableWidgetItem(teamName);
    item->setData(TeamIdxRole, teamIdx);
    return item;
}

// Creates a normal-looking item that has player + team link data attached.
inline QTableWidgetItem* makePlayerLink(const QString& playerName, int teamIdx, int squadIdx) {
    auto* item = new QTableWidgetItem(playerName);
    item->setData(TeamIdxRole, teamIdx);
    item->setData(PlayerIdxRole, squadIdx);
    return item;
}

// ============================================================================
// HOVER HANDLER
// ============================================================================
// Event filter that watches mouse movement on a QTableWidget's viewport.
// When the mouse enters a link cell: accent green + underline + pointing hand.
// When it leaves: restore original appearance.
class HoverHandler : public QObject {
public:
    explicit HoverHandler(QTableWidget* table) : QObject(table), table(table) {
        table->viewport()->setMouseTracking(true);
        table->viewport()->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        if (obj != table->viewport()) return false;

        if (event->type() == QEvent::MouseMove) {
            auto* me = static_cast<QMouseEvent*>(event);
            auto* item = table->itemAt(me->pos());

            if (item == hoveredItem) return false;  // Same cell, nothing to do

            // Un-hover the previous cell (check it's still in the table — refresh may have deleted it)
            if (hoveredItem && table->row(hoveredItem) >= 0 && isLink(hoveredItem)) {
                hoveredItem->setForeground(savedForeground);
                QFont f = hoveredItem->font();
                f.setUnderline(false);
                hoveredItem->setFont(f);
            }

            // Hover the new cell if it's a link
            if (item && isLink(item)) {
                savedForeground = item->foreground();
                item->setForeground(QColor("#2dcc73"));
                QFont f = item->font();
                f.setUnderline(true);
                item->setFont(f);
                table->viewport()->setCursor(Qt::PointingHandCursor);
            } else {
                table->viewport()->unsetCursor();
            }

            hoveredItem = item;
        }
        else if (event->type() == QEvent::Leave) {
            // Mouse left the table entirely — clean up any hover state
            if (hoveredItem && table->row(hoveredItem) >= 0 && isLink(hoveredItem)) {
                hoveredItem->setForeground(savedForeground);
                QFont f = hoveredItem->font();
                f.setUnderline(false);
                hoveredItem->setFont(f);
            }
            hoveredItem = nullptr;
            table->viewport()->unsetCursor();
        }

        return false;  // Don't consume the event — let Qt process it normally
    }

private:
    QTableWidget* table;
    QTableWidgetItem* hoveredItem = nullptr;
    QBrush savedForeground;
};

// One-liner to install hover behavior on a table. Call once in the widget constructor.
inline void installHoverHandler(QTableWidget* table) {
    new HoverHandler(table);  // Parented to the table, cleaned up automatically
}

} // namespace LinkUtils
