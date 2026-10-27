cmake_minimum_required(VERSION 3.24)

project(SaveGuard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(MSVC)
    add_compile_options(/W3 /permissive- /Zc:__cplusplus /utf-8)
else()
    add_compile_options(-Wall -Wextra)
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Test)
qt_standard_project_setup()

set(SG_SOURCES
    src/main.cpp
    src/core/Hasher.cpp
    src/core/Scanner.cpp
    src/core/DuplicateDetector.cpp
    src/core/CleanupManager.cpp
    src/core/HistoryStore.cpp
    src/core/Settings.cpp
    src/core/ProtectedPaths.cpp
    src/core/I18n.cpp
    src/core/DrivesInfo.cpp
    src/platform/PlatformHelper.cpp
    src/ui/Theme.cpp
    src/ui/MainWindow.cpp
    src/ui/DashboardPage.cpp
    src/ui/DuplicatesPage.cpp
    src/ui/BigFilesPage.cpp
    src/ui/HistoryPage.cpp
    src/ui/SettingsPage.cpp
    src/ui/ReviewDialog.cpp
)

qt_add_executable(SaveGuard WIN32
    ${SG_SOURCES}
    resources/resources.qrc
)

target_include_directories(SaveGuard PRIVATE src src/core src/ui src/platform)

target_link_libraries(SaveGuard PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

qt_add_executable(SaveGuardTests
    tests/TestCore.cpp
    src/core/Hasher.cpp
    src/core/Scanner.cpp
    src/core/DuplicateDetector.cpp
    src/core/CleanupManager.cpp
    src/core/HistoryStore.cpp
    src/core/Settings.cpp
    src/core/ProtectedPaths.cpp
    src/core/DrivesInfo.cpp
)

target_include_directories(SaveGuardTests PRIVATE src src/core)

target_link_libraries(SaveGuardTests PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Test
)

enable_testing()
add_test(NAME SaveGuardTests COMMAND SaveGuardTests)
