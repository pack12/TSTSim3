// TacticsWidget.cpp
// Implementation of the tactics screen.
// Three dropdowns that directly modify the team's Tactics struct.
// Changes are immediate — no "Apply" button needed.

#include "TacticsWidget.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>

TacticsWidget::TacticsWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    auto* title = new QLabel("Tactics");
    title->setFont(QFont("Helvetica", 16, QFont::Bold));
    layout->addWidget(title);

    // Group box with form layout: label on the left, dropdown on the right
    auto* group = new QGroupBox("Team Setup");
    auto* form = new QFormLayout(group);

    // Formation dropdown — the order matches the Formation enum exactly
    // (F_4_4_2 = 0, F_4_3_3 = 1, etc.)
    formationCombo = new QComboBox;
    formationCombo->addItems({"4-4-2", "4-3-3", "3-5-2", "4-5-1", "3-4-3", "5-3-2"});
    form->addRow("Formation:", formationCombo);

    // Mentality dropdown — also matches the Mentality enum order
    mentalityCombo = new QComboBox;
    mentalityCombo->addItems({"Very Defensive", "Defensive", "Balanced", "Attacking", "Very Attacking"});
    form->addRow("Mentality:", mentalityCombo);

    // Play style dropdown — matches PlayStyle enum order
    playStyleCombo = new QComboBox;
    playStyleCombo->addItems({"Park the Bus", "Counter Attack", "Balanced", "Possession", "Direct", "All Out Attack"});
    form->addRow("Play Style:", playStyleCombo);

    layout->addWidget(group);

    // Summary label at the bottom: shows the calculated modifier values
    summaryLabel = new QLabel;
    summaryLabel->setFont(QFont("Helvetica", 12));
    summaryLabel->setWordWrap(true);
    layout->addWidget(summaryLabel);

    layout->addStretch();

    auto* backBtn = new QPushButton("Back");
    backBtn->setFixedWidth(100);
    connect(backBtn, &QPushButton::clicked, this, &TacticsWidget::backClicked);
    layout->addWidget(backBtn);

    // Connect dropdown changes to handlers.
    // We use the QOverload template because QComboBox::currentIndexChanged has two overloads
    // (one passing int, one passing QString) and we need to specify which one we want.
    connect(formationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TacticsWidget::onFormationChanged);
    connect(mentalityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TacticsWidget::onMentalityChanged);
    connect(playStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TacticsWidget::onPlayStyleChanged);
}

// ============================================================================
// REFRESH
// ============================================================================
// Sets the dropdowns to match the team's current tactics and updates the modifier summary.
//
// IMPORTANT: We blockSignals on all combos before setting their values.
// Without this, calling setCurrentIndex() would trigger onFormationChanged() etc.,
// which calls refresh() again, creating an infinite loop.
void TacticsWidget::refresh(Team& team) {
    currentTeam = &team;

    // Block signals to prevent cascade: we're setting values, not the user
    formationCombo->blockSignals(true);
    mentalityCombo->blockSignals(true);
    playStyleCombo->blockSignals(true);

    // Set dropdown values to match current tactics.
    // The enum values map directly to dropdown indices (both are 0-based in the same order).
    formationCombo->setCurrentIndex((int)team.tactics.formation);
    mentalityCombo->setCurrentIndex((int)team.tactics.mentality);
    playStyleCombo->setCurrentIndex((int)team.tactics.playStyle);

    // Unblock signals so the user's future selections will trigger handlers
    formationCombo->blockSignals(false);
    mentalityCombo->blockSignals(false);
    playStyleCombo->blockSignals(false);

    // Update the summary label showing all four modifier values
    auto& t = team.tactics;
    summaryLabel->setText(QString(
        "Attack modifier: %1x  |  Defense modifier: %2x  |  "
        "Midfield modifier: %3x  |  Tempo: %4x")
        .arg(t.attackModifier(), 0, 'f', 2)
        .arg(t.defenseModifier(), 0, 'f', 2)
        .arg(t.midfieldModifier(), 0, 'f', 2)
        .arg(t.tempoModifier(), 0, 'f', 2));
}

// Dropdown change handlers: cast the selected index back to the enum type,
// update the team's tactics, and refresh the display to show new modifier values.

void TacticsWidget::onFormationChanged(int index) {
    if (currentTeam) {
        currentTeam->tactics.formation = (Formation)index;
        refresh(*currentTeam);
    }
}

void TacticsWidget::onMentalityChanged(int index) {
    if (currentTeam) {
        currentTeam->tactics.mentality = (Mentality)index;
        refresh(*currentTeam);
    }
}

void TacticsWidget::onPlayStyleChanged(int index) {
    if (currentTeam) {
        currentTeam->tactics.playStyle = (PlayStyle)index;
        refresh(*currentTeam);
    }
}
