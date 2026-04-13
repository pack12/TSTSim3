// PlayerProfileWidget.cpp
// Implementation of the player profile screen.
//
// ATTRIBUTE BARS:
// Each attribute (Pace, Passing, Shooting, etc.) is displayed as a horizontal
// QProgressBar with a color that reflects the value:
//   - Green:  80%+ of max (value 16-20) — world class
//   - Yellow-green: 60-80% (value 12-15) — good
//   - Yellow: 40-60% (value 8-11) — average
//   - Orange: 25-40% (value 5-7) — below average
//   - Red:    <25% (value 1-4) — genuinely terrible
//
// The bars are arranged in a 2-column grid (4 rows) for a compact layout.
// They're rebuilt from scratch every time refresh() is called by deleting
// the old children and creating new ones.

#include "PlayerProfileWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QFrame>

PlayerProfileWidget::PlayerProfileWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    // Player name — big and bold at the top of the screen
    nameLabel = new QLabel;
    nameLabel->setFont(QFont("Helvetica", 22, QFont::Bold));
    layout->addWidget(nameLabel);

    // Info line: position (color-coded), age, overall, club (clickable), value, starter status
    infoLabel = new QLabel;
    infoLabel->setFont(QFont("Helvetica", 13));
    infoLabel->setWordWrap(true);
    infoLabel->setOpenExternalLinks(false);
    connect(infoLabel, &QLabel::linkActivated, this, [this](const QString& link) {
        if (link.startsWith("team:")) {
            int idx = link.mid(5).toInt();
            emit teamClicked(idx);
        }
    });
    layout->addWidget(infoLabel);

    // Horizontal separator line
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep);

    // Container widget for the attribute bar grid (rebuilt on each refresh)
    attrsContainer = new QWidget;
    layout->addWidget(attrsContainer);

    // Another separator before the stats
    auto* sep2 = new QFrame;
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    layout->addWidget(sep2);

    // Season statistics: appearances, goals, assists (shown as an HTML table)
    statsLabel = new QLabel;
    statsLabel->setFont(QFont("Helvetica", 13));
    statsLabel->setWordWrap(true);
    layout->addWidget(statsLabel);

    layout->addStretch();  // Push everything up

    QHBoxLayout* buttonRow = new QHBoxLayout;

    auto* backToSquadBtn = new QPushButton("Back to Squad");
    backToSquadBtn->setFixedWidth(140);
    connect(backToSquadBtn, &QPushButton::clicked, this, &PlayerProfileWidget::backClicked);

    auto* backToPreviousPageBtn = new QPushButton("Back");
    backToPreviousPageBtn->setFixedWidth(140);
    connect(backToPreviousPageBtn, &QPushButton::clicked, this, &PlayerProfileWidget::backToPreviousPageClicked);

    buttonRow->addWidget(backToPreviousPageBtn);
    buttonRow->addWidget(backToSquadBtn);
    buttonRow->setSpacing(5);
    buttonRow->setAlignment(Qt::AlignLeft);


    //layout->addWidget(backBtn);
    layout->addLayout(buttonRow);
}

// ============================================================================
// BUILD ATTRIBUTE BAR
// ============================================================================
// Creates a single attribute display row consisting of:
//   [Label (100px)] [Progress Bar (fills remaining space)] [Value Number (30px)]
//
// The progress bar color is determined by the value/maxVal ratio:
//   >80%: bright green (elite attribute)
//   60-80%: yellow-green (good)
//   40-60%: yellow (average)
//   25-40%: orange (below average)
//   <25%: red (terrible)
QWidget* PlayerProfileWidget::buildAttrBar(const QString& label, int value, int maxVal) {
    auto* row = new QWidget;
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 2, 0, 2);  // Minimal vertical padding between bars

    // Attribute name label (fixed width for alignment)
    auto* nameLabel = new QLabel(label);
    nameLabel->setFixedWidth(100);
    nameLabel->setFont(QFont("Helvetica", 12));
    layout->addWidget(nameLabel);

    // Progress bar — used purely for visual display, not actual "progress"
    auto* bar = new QProgressBar;
    bar->setMinimum(0);
    bar->setMaximum(maxVal);
    bar->setValue(value);
    bar->setTextVisible(false);   // We show the number separately, not inside the bar
    bar->setFixedHeight(18);

    // Determine the bar color based on the ratio of value to max
    QColor color;
    float ratio = (float)value / maxVal;
    if (ratio >= 0.8f) color = QColor(50, 180, 50);       // Bright green — world class
    else if (ratio >= 0.6f) color = QColor(120, 190, 60); // Yellow-green — good
    else if (ratio >= 0.4f) color = QColor(220, 180, 40); // Yellow — average
    else if (ratio >= 0.25f) color = QColor(220, 120, 40);// Orange — below average
    else color = QColor(200, 50, 50);                       // Red — terrible

    // Apply the color via stylesheet (Qt's way of styling widgets)
    bar->setStyleSheet(QString(
        "QProgressBar { background: #1a2d42; border: 1px solid #1e3048; border-radius: 3px; }"
        "QProgressBar::chunk { background: %1; border-radius: 2px; }")
        .arg(color.name()));

    layout->addWidget(bar);

    // Numeric value label at the right end (e.g. "18")
    auto* valLabel = new QLabel(QString::number(value));
    valLabel->setFixedWidth(30);
    valLabel->setAlignment(Qt::AlignCenter);
    valLabel->setFont(QFont("Helvetica", 12, QFont::Bold));
    layout->addWidget(valLabel);

    return row;
}

// ============================================================================
// REFRESH
// ============================================================================
// Populates the profile with data from the given player.
// Called every time the user navigates to this screen from the squad list.
void PlayerProfileWidget::refresh(const Player& player, const std::string& teamName, bool isStarter, int teamIdx) {
    currentTeamIdx = teamIdx;
    nameLabel->setText(QString::fromStdString(player.name));

    // Position gets a color: GK=gold, DEF=blue, MID=green, FWD=red
    // These colors match the general "feel" of each position in football culture.
    QString posColor;
    switch (player.position) {
        case Position::GK:  posColor = "#e8a800"; break;  // Gold
        case Position::DEF: posColor = "#2277cc"; break;  // Blue
        case Position::MID: posColor = "#22aa44"; break;  // Green
        case Position::FWD: posColor = "#cc3333"; break;  // Red
    }

    // Build the info line using HTML for inline styling.
    // The club name is a clickable link styled in accent green.
    QString clubHtml = (teamIdx >= 0)
        ? QString("<a href='team:%1' style='color:#2dcc73; text-decoration:underline; font-weight:bold;'>%2</a>")
            .arg(teamIdx).arg(QString::fromStdString(teamName))
        : QString("<b>%1</b>").arg(QString::fromStdString(teamName));

    QString info = QString(
        "<span style='font-size:15px; color:%1; font-weight:bold;'>%2</span>"
        "  &nbsp;|&nbsp;  Age: <b>%3</b>"
        "  &nbsp;|&nbsp;  Overall: <b>%4</b>"
        "  &nbsp;|&nbsp;  Club: %5"
        "  &nbsp;|&nbsp;  Value: <b>£%6k</b>"
        "  &nbsp;|&nbsp;  %7")
        .arg(posColor)
        .arg(QString::fromStdString(player.positionStr()))
        .arg(player.age)
        .arg(player.overall())
        .arg(clubHtml)
        .arg(player.marketValue() / 1000)
        .arg(isStarter ? "<span style='color:green;'>Starting XI</span>" : "Substitute");

    infoLabel->setText(info);

    // ---- REBUILD ATTRIBUTE BARS ----
    // We need to destroy the old layout and child widgets, then create new ones.
    // This is necessary because the attribute values change between different players.
    delete attrsContainer->layout();  // Remove old grid layout
    // Delete all child widgets of the container
    auto children = attrsContainer->findChildren<QWidget*>(Qt::FindDirectChildrenOnly);
    for (auto* c : children) delete c;

    // Create a 2-column grid of attribute bars
    auto* grid = new QGridLayout(attrsContainer);
    grid->setContentsMargins(0, 5, 0, 5);

    // Define all 8 attributes with their display names and values
    struct Attr { QString name; int value; };
    Attr attrs[] = {
        {"Pace",        player.pace},
        {"Passing",     player.passing},
        {"Shooting",    player.shooting},
        {"Defending",   player.defending},
        {"Dribbling",   player.dribbling},
        {"Goalkeeping", player.goalkeeping},
        {"Stamina",     player.stamina},
        {"Creativity",  player.creativity},
    };

    // Place attribute bars in a 2-column grid (4 rows x 2 columns)
    int row = 0, col = 0;
    for (auto& a : attrs) {
        grid->addWidget(buildAttrBar(a.name, a.value), row, col);
        col++;
        if (col >= 2) { col = 0; row++; }
    }

    // ---- SEASON STATISTICS ----
    // Shown as an HTML table for clean formatting
    QString stats = QString(
        "<h3>Season Statistics</h3>"
        "<table cellpadding='6' cellspacing='0' style='font-size:13px;'>"
        "<tr><td><b>Appearances:</b></td><td>%1</td>"
        "<td width='30'></td>"
        "<td><b>Goals:</b></td><td>%2</td>"
        "<td width='30'></td>"
        "<td><b>Assists:</b></td><td>%3</td></tr>"
        "</table>")
        .arg(player.appearances)
        .arg(player.goals)
        .arg(player.assists);

    // If the player has appearances, show goals per game ratio
    if (player.appearances > 0) {
        float goalsPerGame = (float)player.goals / player.appearances;
        stats += QString("<p style='color:#8b949e;'>Goals per game: %1</p>")
            .arg(goalsPerGame, 0, 'f', 2);
    }

    statsLabel->setText(stats);
}
