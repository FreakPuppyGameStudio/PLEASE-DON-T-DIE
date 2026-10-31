cmake_minimum_required(VERSION 3.16)
project(PleaseDontDie LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Raylib: use an installed copy if present, otherwise download it.
find_package(raylib 4.5 QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
        GIT_SHALLOW TRUE
    )
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(raylib)
endif()

# Headless game logic: no Raylib, shared by the game and the tests.
add_library(core STATIC
    src/narrative/json.cpp
    src/narrative/content.cpp
    src/simulation/simulation.cpp
    src/systems/condition.cpp
    src/systems/time_system.cpp
    src/systems/event_system.cpp
    src/systems/ending_system.cpp
    src/systems/evolution_system.cpp
    src/terminal/text_wrap.cpp
    src/terminal/actions.cpp
    src/terminal/command_parser.cpp
    src/save/save_system.cpp
)
target_include_directories(core PUBLIC src)

# Game executable: Raylib front end and audio.
add_executable(please_dont_die
    src/main.cpp
    src/terminal/terminal_view.cpp
    src/audio/music_player.cpp
)
target_link_libraries(please_dont_die PRIVATE core raylib)

# Copy data/ and audio/ next to the executable so the game runs from anywhere.
add_custom_command(TARGET please_dont_die POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:please_dont_die>/data
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/audio $<TARGET_FILE_DIR:please_dont_die>/audio
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:please_dont_die>/saves
)

# Headless tests.
add_executable(test_logic tests/test_logic.cpp)
target_link_libraries(test_logic PRIVATE core)

enable_testing()
add_test(NAME logic COMMAND test_logic)
