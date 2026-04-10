// Theme.h
// Football Manager-style dark theme stylesheet for the entire application.
// Qt uses a CSS-like stylesheet system — you set one global stylesheet on the
// QApplication and it cascades down to every widget. Individual widgets can
// override specific properties, but the base look comes from here.
//
// COLOR PALETTE:
//   Background:     #0e1621  (dark navy — the base color of everything)
//   Panel/Header:   #152232  (slightly lighter navy — for panels, cards, headers)
//   Sidebar:        #0a1120  (darkest navy — sidebar background)
//   Accent Green:   #1a8a4a  (FM's signature green — active items, primary actions)
//   Accent Bright:  #2dcc73  (brighter green — text highlights, active labels)
//   Text Primary:   #c8d6e5  (light gray-blue — main readable text)
//   Text Secondary: #8b949e  (muted gray — labels, less important text)
//   Border:         #1e3048  (subtle navy border — separators, outlines)
//   Hover:          #1e3a52  (slightly lighter — hover state for interactive elements)
//   Button:         #1a2d42  (dark blue — default button background)
//   Table Alt Row:  #12202e  (very subtle stripe for alternating table rows)

#pragma once
#include <QString>

namespace Theme {

inline QString globalStylesheet() {
    return R"(

    /* ===== GLOBAL DEFAULTS ===== */
    QMainWindow, QWidget {
        background-color: #0e1621;
        color: #c8d6e5;
    }

    QLabel {
        color: #c8d6e5;
        background: transparent;
    }

    /* ===== BUTTONS ===== */
    QPushButton {
        background-color: #1a2d42;
        color: #c8d6e5;
        border: 1px solid #1e3048;
        border-radius: 4px;
        padding: 6px 14px;
        font-size: 12px;
    }
    QPushButton:hover {
        background-color: #243d56;
        border-color: #2a5070;
    }
    QPushButton:pressed {
        background-color: #0f1f30;
    }
    QPushButton:disabled {
        background-color: #111b28;
        color: #3a4a5a;
        border-color: #162030;
    }

    /* ===== TABLES ===== */
    QTableWidget {
        background-color: #0e1621;
        alternate-background-color: #12202e;
        gridline-color: #1e3048;
        border: 1px solid #1e3048;
        color: #c8d6e5;
        selection-background-color: #1a3a2a;
        selection-color: #ffffff;
    }
    QTableWidget::item {
        padding: 3px;
    }
    QTableWidget::item:selected {
        background-color: #1a3a2a;
        color: #ffffff;
    }
    QHeaderView::section {
        background-color: #1a2d42;
        color: #8b949e;
        border: 1px solid #1e3048;
        padding: 5px;
        font-weight: bold;
    }

    /* ===== SCROLL BARS ===== */
    QScrollBar:vertical {
        background: #0e1621;
        width: 10px;
        border: none;
    }
    QScrollBar::handle:vertical {
        background: #1e3048;
        border-radius: 5px;
        min-height: 20px;
    }
    QScrollBar::handle:vertical:hover {
        background: #2a5070;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
        background: none;
        height: 0px;
    }
    QScrollBar:horizontal {
        background: #0e1621;
        height: 10px;
        border: none;
    }
    QScrollBar::handle:horizontal {
        background: #1e3048;
        border-radius: 5px;
        min-width: 20px;
    }
    QScrollBar::handle:horizontal:hover {
        background: #2a5070;
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal,
    QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
        background: none;
        width: 0px;
    }

    /* ===== COMBO BOX ===== */
    QComboBox {
        background-color: #1a2d42;
        color: #c8d6e5;
        border: 1px solid #1e3048;
        border-radius: 4px;
        padding: 5px 8px;
        min-height: 20px;
    }
    QComboBox:hover {
        border-color: #2a5070;
    }
    QComboBox::drop-down {
        border: none;
        width: 20px;
    }
    QComboBox::down-arrow {
        image: none;
        border-left: 4px solid transparent;
        border-right: 4px solid transparent;
        border-top: 6px solid #8b949e;
        margin-right: 6px;
    }
    QComboBox QAbstractItemView {
        background-color: #1a2d42;
        color: #c8d6e5;
        selection-background-color: #1a8a4a;
        selection-color: #ffffff;
        border: 1px solid #1e3048;
    }

    /* ===== SPIN BOX ===== */
    QSpinBox {
        background-color: #1a2d42;
        color: #c8d6e5;
        border: 1px solid #1e3048;
        border-radius: 4px;
        padding: 4px;
    }
    QSpinBox::up-button, QSpinBox::down-button {
        background: #243d56;
        border: 1px solid #1e3048;
        width: 16px;
    }
    QSpinBox::up-button:hover, QSpinBox::down-button:hover {
        background: #2a5070;
    }

    /* ===== TEXT EDIT ===== */
    QTextEdit {
        background-color: #12202e;
        color: #c8d6e5;
        border: 1px solid #1e3048;
        selection-background-color: #1a3a2a;
    }

    /* ===== LIST WIDGET ===== */
    QListWidget {
        background-color: #0e1621;
        alternate-background-color: #12202e;
        color: #c8d6e5;
        border: 1px solid #1e3048;
        outline: none;
    }
    QListWidget::item {
        padding: 6px;
    }
    QListWidget::item:selected {
        background-color: #1a3a2a;
        color: #ffffff;
    }
    QListWidget::item:hover {
        background-color: #1e3a52;
    }

    /* ===== GROUP BOX ===== */
    QGroupBox {
        color: #c8d6e5;
        border: 1px solid #1e3048;
        border-radius: 4px;
        margin-top: 10px;
        padding-top: 14px;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        padding: 0 8px;
        color: #8b949e;
    }

    /* ===== SPLITTER ===== */
    QSplitter::handle {
        background: #1e3048;
        width: 2px;
    }

    /* ===== STATUS BAR ===== */
    QStatusBar {
        background-color: #0a1120;
        color: #6b7c93;
        border-top: 1px solid #1e3048;
        font-size: 11px;
    }

    /* ===== MESSAGE BOX ===== */
    QMessageBox {
        background-color: #152232;
    }
    QMessageBox QLabel {
        color: #c8d6e5;
        min-width: 250px;
    }
    QMessageBox QPushButton {
        min-width: 80px;
        padding: 6px 16px;
    }

    /* ===== PROGRESS BAR (used in player profile) ===== */
    QProgressBar {
        background: #1a2d42;
        border: 1px solid #1e3048;
        border-radius: 3px;
        text-align: center;
        color: #c8d6e5;
    }

    /* ===== FORM LAYOUT LABELS ===== */
    QFormLayout QLabel {
        color: #8b949e;
    }

    /* ===== FRAME SEPARATORS ===== */
    QFrame[frameShape="4"],
    QFrame[frameShape="5"] {
        color: #1e3048;
        background: #1e3048;
        max-height: 1px;
    }

    /* =============================================== */
    /* CUSTOM WIDGET STYLES (using object names)       */
    /* =============================================== */

    /* --- TOP BAR --- */
    QWidget#topBar {
        background-color: #152232;
        border-bottom: 1px solid #1e3048;
    }

    /* --- SIDEBAR --- */
    QWidget#sidebar {
        background-color: #0a1120;
        border-right: 1px solid #1e3048;
    }

    /* --- SIDEBAR NAV BUTTONS --- */
    QPushButton#sidebarBtn {
        background-color: transparent;
        color: #6b7c93;
        border: none;
        border-left: 3px solid transparent;
        border-radius: 0px;
        text-align: left;
        padding: 10px 16px 10px 14px;
        font-size: 13px;
    }
    QPushButton#sidebarBtn:hover {
        background-color: #111d2e;
        color: #c8d6e5;
    }
    QPushButton#sidebarBtn[active="true"] {
        background-color: #111d2e;
        color: #2dcc73;
        border-left: 3px solid #1a8a4a;
    }
    QPushButton#sidebarBtn:disabled {
        color: #2a3a4a;
    }

    /* --- PLAY MATCH BUTTON (green accent) --- */
    QPushButton#playMatchBtn {
        background-color: #1a8a4a;
        color: #ffffff;
        border: none;
        border-radius: 4px;
        padding: 10px 16px;
        font-size: 13px;
        font-weight: bold;
    }
    QPushButton#playMatchBtn:hover {
        background-color: #22a85a;
    }
    QPushButton#playMatchBtn:pressed {
        background-color: #147038;
    }
    QPushButton#playMatchBtn:disabled {
        background-color: #0f3020;
        color: #3a5a4a;
    }

    /* --- SAVE BUTTON (subtle) --- */
    QPushButton#saveBtn {
        background-color: transparent;
        color: #6b7c93;
        border: 1px solid #1e3048;
        border-radius: 4px;
        padding: 8px 16px;
        font-size: 12px;
    }
    QPushButton#saveBtn:hover {
        background-color: #152232;
        color: #c8d6e5;
    }
    QPushButton#saveBtn:disabled {
        color: #2a3a4a;
        border-color: #162030;
    }

    /* --- TITLE SCREEN PRIMARY BUTTON --- */
    QPushButton#titleBtnPrimary {
        background-color: #1a8a4a;
        color: #ffffff;
        border: none;
        border-radius: 6px;
        padding: 10px 20px;
        font-size: 14px;
        font-weight: bold;
    }
    QPushButton#titleBtnPrimary:hover {
        background-color: #22a85a;
    }

    /* --- TITLE SCREEN SECONDARY BUTTON --- */
    QPushButton#titleBtn {
        background-color: #1a2d42;
        color: #c8d6e5;
        border: 1px solid #1e3048;
        border-radius: 6px;
        padding: 10px 20px;
        font-size: 14px;
    }
    QPushButton#titleBtn:hover {
        background-color: #243d56;
        border-color: #1a8a4a;
    }

    )";
}

} // namespace Theme
