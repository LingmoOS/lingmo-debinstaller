# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(lingmo-debinstaller VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

find_package(LingmoSDK REQUIRED)

add_executable(lingmo-debinstaller
    src/debbackend.cpp
    src/deb_window.cpp
    src/main.cpp
)
target_link_libraries(lingmo-debinstaller PRIVATE Lingmo::SDK)
target_compile_definitions(lingmo-debinstaller PRIVATE
    DEBINSTALLER_VERSION="${PROJECT_VERSION}")

install(TARGETS lingmo-debinstaller RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES icons/lingmo-debinstaller.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
lingmo_add_translations(lingmo-debinstaller SOURCES src
    DESKTOP_FILES lingmo-debinstaller.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-debinstaller.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
