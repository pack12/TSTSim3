cmake_minimum_required(VERSION 3.16)
project(TST3SoccerSim VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(TST3SoccerSim
    src/main.cpp
    src/Player.cpp
    src/Team.cpp
    src/Tactics.cpp
    src/League.cpp
    src/MatchEngine.cpp
    src/TransferMarket.cpp
    src/SaveLoad.cpp
    src/MainWindow.cpp
    src/SquadWidget.cpp
    src/LeagueTableWidget.cpp
    src/TacticsWidget.cpp
    src/MatchWidget.cpp
    src/TransferWidget.cpp
    src/TopScorersWidget.cpp
    src/FixturesWidget.cpp
    src/PlayerProfileWidget.cpp
)

target_include_directories(TST3SoccerSim PRIVATE src)
target_link_libraries(TST3SoccerSim PRIVATE Qt6::Widgets)
