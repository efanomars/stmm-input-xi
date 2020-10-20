# Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, see <http://www.gnu.org/licenses/>

# File:   device-floater/CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

option(STMM_INSTALL_MAN_PAGE "Install man page in debian.orig/" ON)
option(STMM_INSTALL_LAUNCHER "Install launcher in share/applications/ (implies STMM_INSTALL_ICONS=ON)" ON)
option(STMM_INSTALL_ICONS "Install icons in share/icons/hicolor/(size)/apps/" ON)


project(device-floater CXX)

set(RUNTIME_OUTPUT_DIRECTORY "build")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../share/cmake")

include(CommonUtil)

CheckBinaryNotSourceTree()
CheckBuildType()
DefineCommonCompileOptions("c++14")

# Source files (and headers only used for building)
set(STMMI_DEVFLO_SOURCES
        "${PROJECT_SOURCE_DIR}/src/config.h"
        "${PROJECT_SOURCE_DIR}/src/devicefloater.h"
        "${PROJECT_SOURCE_DIR}/src/devicefloater.cc"
        "${PROJECT_SOURCE_DIR}/src/main.cc"
        "${PROJECT_SOURCE_DIR}/src/xideviceswindow.h"
        "${PROJECT_SOURCE_DIR}/src/xideviceswindow.cc"
        )
set(STMMI_DEVFLO_DATA_DIR  "${PROJECT_SOURCE_DIR}/data")
set(STMMI_DEVFLO_DATA_FILES
        "${STMMI_DEVFLO_DATA_DIR}/images/buoy_32313.png"
        "${STMMI_DEVFLO_DATA_DIR}/images/keyboard_1420.png"
        "${STMMI_DEVFLO_DATA_DIR}/images/mouse_347.png"
        )

add_executable(device-floater ${STMMI_DEVFLO_SOURCES} "${PROJECT_BINARY_DIR}/config.cc")

include("device-floater-defs.cmake")

target_include_directories(device-floater SYSTEM PUBLIC ${DEVICEFLOATER_EXTRA_INCLUDE_DIRS})
# This allows config.cc to find the config.h include
target_include_directories(device-floater        PUBLIC "${PROJECT_SOURCE_DIR}/src")

target_link_libraries(device-floater ${DEVICEFLOATER_EXTRA_LIBRARIES})

DefineTargetPublicCompileOptions(device-floater)

include(GNUInstallDirs)
set(STMMI_DEVFLO_PKG_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/device-floater")
set(STMMI_DEVFLO_PKG_REL_DATA_DIR  "${CMAKE_INSTALL_DATADIR}/device-floater")

# Create config file for executable
configure_file("${PROJECT_SOURCE_DIR}/src/config.cc.in"
               "${PROJECT_BINARY_DIR}/config.cc" @ONLY)
if (STMM_INSTALL_MAN_PAGE)
    configure_file("${PROJECT_SOURCE_DIR}/../scripts/priv/debian.orig/device-floater.1"
                   "${PROJECT_BINARY_DIR}/device-floater.1" @ONLY)
    CreateManGz(device-floater "device-floater.1")
endif()

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "device-floater was configured with the following options:")
message(STATUS " STMMI_DEVFLO_SOURCES:             ${STMMI_DEVFLO_SOURCES}")
message(STATUS " DEVICEFLOATER_EXTRA_INCLUDE_DIRS: ${DEVICEFLOATER_EXTRA_INCLUDE_DIRS}")
message(STATUS " DEVICEFLOATER_EXTRA_LIBRARIES:    ${DEVICEFLOATER_EXTRA_LIBRARIES}")
message(STATUS " DEVICEFLOATER_DATA_FILES          ${DEVICEFLOATER_DATA_FILES}")
message(STATUS " install prefix:                   ${CMAKE_INSTALL_PREFIX}")
message(STATUS " CMAKE_BUILD_TYPE:                 ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:            ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:       ${CMAKE_CXX_COMPILER_VERSION}")
endif()

install(TARGETS device-floater RUNTIME DESTINATION "bin")

install(FILES ${STMMI_DEVFLO_DATA_FILES} DESTINATION "${STMMI_DEVFLO_PKG_REL_DATA_DIR}/images")

if (STMM_INSTALL_LAUNCHER)
    install(FILES        "${STMMI_DEVFLO_DATA_DIR}/applications/com.efanomars.device-floater.desktop"
            DESTINATION  "${CMAKE_INSTALL_DATADIR}/applications/")
endif()
if (STMM_INSTALL_ICONS OR STMM_INSTALL_LAUNCHER)
    install(FILES          "${STMMI_DEVFLO_DATA_DIR}/icons/hicolor/24x24/apps/device-floater.png"
            DESTINATION    "${CMAKE_INSTALL_DATADIR}/icons/hicolor/24x24/apps/")
    install(FILES          "${STMMI_DEVFLO_DATA_DIR}/icons/hicolor/32x32/apps/device-floater.png"
            DESTINATION    "${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps/")
    install(FILES          "${STMMI_DEVFLO_DATA_DIR}/icons/hicolor/48x48/apps/device-floater.png"
            DESTINATION    "${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps/")
    install(FILES       "${STMMI_DEVFLO_DATA_DIR}/icons/hicolor/scalable/apps/device-floater.svg"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/")
endif()
if (STMM_INSTALL_MAN_PAGE)
    install(FILES                   "${PROJECT_BINARY_DIR}/device-floater.1.gz"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/man/man1/")
endif()
