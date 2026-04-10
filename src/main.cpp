// main.cpp
// Entry point for the TST3 Soccer Manager application.
// This is about as simple as a Qt application gets:
//   1. Create a QApplication (Qt's main event loop handler)
//   2. Create our MainWindow (which sets up all the screens/widgets)
//   3. Show the window
//   4. Run the event loop until the user closes the application
//
// Everything interesting happens inside MainWindow. This file just boots it up.

#include "MainWindow.h"
#include <QApplication>


int main(int argc, char* argv[]) {
    // QApplication manages the GUI application's control flow and main settings.
    // argc and argv are passed through so Qt can process command-line arguments
    // (like -style, -geometry, etc. — built-in Qt flags we don't need to handle ourselves).
    QApplication app(argc, argv);
    app.setApplicationName("TST3 Soccer Manager");

    // Create and show the main window. The constructor builds all screens,
    // wires up all signal/slot connections, and sets up the game state.
    MainWindow window;
    window.show();

    // Enter the event loop. This blocks until the window is closed.
    // app.exec() returns 0 on normal exit.
    return app.exec();
}
