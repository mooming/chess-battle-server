cmake_minimum_required(VERSION 3.10)
project(ChessBattleServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Server executable ──────────────────────────────────────────────────
add_executable(Server
    ChessJam/Main.cpp
    ChessJam/CommandLineInput.cpp
    ChessJam/ExecutionMode.cpp
    ChessJam/StringHelper.cpp
    ChessJam/Server.cpp
    ChessJam/Session/SessionManager.cpp
    ChessJam/Engine/FenParser.cpp
    ChessJam/Engine/Rules.cpp
)

target_include_directories(Server PRIVATE ChessJam)
target_link_libraries(Server PRIVATE pthread)

# ── Test client executable ─────────────────────────────────────────────
add_executable(Client
    ChessJam/Client/main.cpp
    ChessJam/Client/ClientStub.cpp
)

target_include_directories(Client PRIVATE ChessJam)
target_link_libraries(Client PRIVATE pthread)

add_executable(Tests
    ChessJam/Tests/TestMain.cpp
    ChessJam/Client/ClientStub.cpp
    ChessJam/Engine/FenParser.cpp
    ChessJam/Engine/Rules.cpp
)

target_include_directories(Tests PRIVATE ChessJam)
target_link_libraries(Tests PRIVATE pthread)

add_test(NAME AllTests COMMAND Tests)
enable_testing()
