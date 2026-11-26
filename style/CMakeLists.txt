# FreeLook: a Qt 6 widget style inspired by OPEN LOOK.
#
#   cmake -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build -j
#   cmake --install build          # installs into ~/.local/lib/qt6/plugins/styles
#
# SPDX-FileCopyrightText: 2026 Saulo Popov Zambiasi <saulo.popov.zambiasi@ufsc.br>
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)
project(freelook-style VERSION 0.7 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)

# Install for the current user by default, no root needed.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install prefix" FORCE)
endif()
set(FREELOOK_PLUGIN_DIR "lib/qt6/plugins/styles" CACHE STRING "Style plugin directory, relative to the prefix")

add_library(freelook MODULE
    src/freelookstyle.cpp
    src/freelookstyle.h
    src/freelookplugin.cpp
)
target_link_libraries(freelook PRIVATE Qt6::Widgets)
install(TARGETS freelook LIBRARY DESTINATION ${FREELOOK_PLUGIN_DIR})

# Widget gallery used to test the style (renders a PNG and runs checks).
option(FREELOOK_GALLERY "Build the test gallery" ON)
if(FREELOOK_GALLERY)
    add_executable(freelook-gallery gallery/gallery.cpp)
    target_link_libraries(freelook-gallery PRIVATE Qt6::Widgets)
endif()
