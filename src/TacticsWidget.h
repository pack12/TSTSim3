// TacticsWidget.h
// The tactics management screen — where you set your team's formation, mentality, and play style.
//
// Uses three QComboBox dropdowns:
//   - Formation: 4-4-2, 4-3-3, 3-5-2, 4-5-1, 3-4-3, 5-3-2
//   - Mentality: Very Defensive → Very Attacking
//   - Play Style: Park the Bus → All Out Attack
//
// Changes take effect IMMEDIATELY — as soon as you select a new option from any dropdown,
// the team's tactics are updated and the modifier summary at the bottom recalculates.
// There's no "Apply" button; everything is live.
//
// The refresh() method uses blockSignals(true/false) to prevent a feedback loop:
// when we programmatically set the dropdown values to match the current tactics,
// we don't want that to trigger onFormationChanged() etc., because that would
// call refresh() again and create an infinite loop.

#pragma once
#include "Team.h"
#include <QWidget>
#include <QComboBox>
#include <QLabel>

class TacticsWidget : public QWidget {
    Q_OBJECT
public:
    explicit TacticsWidget(QWidget* parent = nullptr);
    void refresh(Team& team);

private:
    // Slot handlers for when the user changes a dropdown selection
    void onFormationChanged(int index);
    void onMentalityChanged(int index);
    void onPlayStyleChanged(int index);

    QComboBox* formationCombo = nullptr;    // Formation dropdown
    QComboBox* mentalityCombo = nullptr;    // Mentality dropdown
    QComboBox* playStyleCombo = nullptr;    // Play style dropdown
    QLabel* summaryLabel = nullptr;         // Shows the calculated modifier values

    Team* currentTeam = nullptr;            // Pointer to the team we're editing
};
