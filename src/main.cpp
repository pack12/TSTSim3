// main.cpp
// Entry point for the TST3 Soccer Manager application.
// Sets up the global dark theme stylesheet and launches the main window.

#include "MainWindow.h"
#include "Theme.h"
#include <QApplication>
#include <QFont>


int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("TST3 Soccer Manager");

    // Set the global font — clean sans-serif for the FM look
    app.setFont(QFont("Helvetica Neue", 11));

    // Apply the dark theme stylesheet to the entire application.
    // This cascades down to every widget — tables, buttons, labels, everything.
    app.setStyleSheet(Theme::globalStylesheet());

    MainWindow window;
    window.show();

    return app.exec();
}
