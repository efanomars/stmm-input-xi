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

# File:   libstmm-input-gtk-xi/CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

project(stmm-input-gtk-xi CXX)

set(LIBRARY_OUTPUT_DIRECTORY "build")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../share/cmake")

include(CommonUtil)

CheckBinaryNotSourceTree()
CheckBuildType()
DefineSharedLibOption()
DefineCommonOptions()
DefineCommonCompileOptions("c++14")

# Headers dir
set(STMMI_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/include")
set(STMMI_HEADERS_DIR  "${STMMI_INCLUDE_DIR}/stmm-input-gtk-xi")

set(STMMI_HEADERS
        "${STMMI_HEADERS_DIR}/flogtkdevicemanager.h"
        "${STMMI_HEADERS_DIR}/stmm-input-gtk-xi.h"
        "${STMMI_HEADERS_DIR}/stmm-input-gtk-xi-config.h"
        )
if (BUILD_SHARED_LIBS)
    set(STMMI_HEADERS  ${STMMI_HEADERS}
        "${STMMI_HEADERS_DIR}/libmain.h"
        )
endif()
#
# Sources dir
set(STMMI_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/src")
# Source files (and headers only used for building)
set(STMMI_SOURCES
        "${STMMI_SOURCES_DIR}/floatingsources.h"
        "${STMMI_SOURCES_DIR}/floatingsources.cc"
        "${STMMI_SOURCES_DIR}/flogtkbackend.h"
        "${STMMI_SOURCES_DIR}/flogtkbackend.cc"
        "${STMMI_SOURCES_DIR}/flogtkdevicemanager.cc"
        "${STMMI_SOURCES_DIR}/flogtklistenerextradata.h"
        "${STMMI_SOURCES_DIR}/flogtklistenerextradata.cc"
        "${STMMI_SOURCES_DIR}/flogtkwindowdata.h"
        "${STMMI_SOURCES_DIR}/flogtkwindowdata.cc"
        "${STMMI_SOURCES_DIR}/flogtkxkeyboarddevice.h"
        "${STMMI_SOURCES_DIR}/flogtkxkeyboarddevice.cc"
        "${STMMI_SOURCES_DIR}/recycler.h"
        "${STMMI_SOURCES_DIR}/recycler.cc"
        "${STMMI_SOURCES_DIR}/stmm-input-gtk-xi.cc"
        )
if (BUILD_SHARED_LIBS)
    set(STMMI_SOURCES
        ${STMMI_SOURCES}
        "${STMMI_SOURCES_DIR}/libmain.cc"
        )
endif()

# Define library
add_library(stmm-input-gtk-xi ${STMMI_SOURCES} "${PROJECT_BINARY_DIR}/stmm-input-gtk-xi-config.cc")

include("stmm-input-gtk-xi-defs.cmake")

target_include_directories(stmm-input-gtk-xi SYSTEM PUBLIC ${STMMINPUTGTKXI_EXTRA_INCLUDE_DIRS})
target_include_directories(stmm-input-gtk-xi BEFORE PUBLIC
  $<BUILD_INTERFACE:${STMMI_HEADERS_DIR}>
  $<INSTALL_INTERFACE:include/stmm-input-gtk-xi>  # <prefix>/include/stmm-input-gtk-xi
)

target_link_libraries(stmm-input-gtk-xi ${STMMINPUTGTKXI_EXTRA_LIBRARIES})

set_target_properties(stmm-input-gtk-xi PROPERTIES  ${CMAKE_BUILD_TYPE}_POSTFIX "")
set_target_properties(stmm-input-gtk-xi PROPERTIES
        VERSION ${STMM_INPUT_GTK_XI_VERSION}
        SOVERSION ${STMM_INPUT_GTK_XI_MAJOR_VERSION}
#        OUTPUT_NAME "stmm-input-gtk-xi-${STMM_INPUT_GTK_XI_VERSION}"
        )

DefineTargetPublicCompileOptions(stmm-input-gtk-xi)

# TODO only if plugin => create option COMPILE_AS_PLUGIN=ON (default)
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") AND (BUILD_SHARED_LIBS))
    target_compile_options(stmm-input-gtk-xi PUBLIC "-Wl,-E")
endif()

# Set version for stmm-input-gtk-xi-config.cc.in
set(STMMI_PKG_VERSION "${STMM_INPUT_GTK_XI_VERSION}")
set(STMMI_PKG_PLUGIN_NAME "90stmm-input-gtk-xi")
# Create config file for library
configure_file("${PROJECT_SOURCE_DIR}/src/stmm-input-gtk-xi-config.cc.in"
               "${PROJECT_BINARY_DIR}/stmm-input-gtk-xi-config.cc" @ONLY)

# Set install dir for pkg-config file
set(STMMI_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
# Create pkg-config file for the library
configure_file("${PROJECT_SOURCE_DIR}/stmm-input-gtk-xi.pc.in"
               "${PROJECT_BINARY_DIR}/stmm-input-gtk-xi.pc" @ONLY)

include(GNUInstallDirs)
set(STMMI_PLUGINS_DATA_DIR      "${CMAKE_INSTALL_FULL_SYSCONFDIR}/stmm-input-dl")
set(STMMI_PLUGINS_USER_DATA_DIR ".local/${CMAKE_INSTALL_SYSCONFDIR}/stmm-input-dl") # not used

set(STMMI_GTK_XI_INSTALL_LIB_PATH1 "${CMAKE_INSTALL_PREFIX}/lib/libstmm-input-gtk-xi.so.${STMM_INPUT_GTK_XI_MAJOR_VERSION}")
set(STMMI_GTK_XI_INSTALL_LIB_PATH2 "/usr/local/lib/libstmm-input-gtk-xi.so.${STMM_INPUT_GTK_XI_MAJOR_VERSION}")
set(STMMI_GTK_XI_INSTALL_LIB_PATH3 "/usr/lib/libstmm-input-gtk-xi.so.${STMM_INPUT_GTK_XI_MAJOR_VERSION}")
set(STMMI_GTK_XI_INSTALL_LIB_PATH4 "libstmm-input-gtk-xi-so.${STMM_INPUT_GTK_XI_MAJOR_VERSION}")

configure_file("${PROJECT_SOURCE_DIR}/${STMMI_PKG_PLUGIN_NAME}.dlp.in"
               "${PROJECT_BINARY_DIR}/${STMMI_PKG_PLUGIN_NAME}.dlp" @ONLY)

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "stmm-input-gtk-xi was configured with the following options:")
message(STATUS " STMMI_HEADERS:                     ${STMMI_HEADERS}")
message(STATUS " STMMI_SOURCES:                     ${STMMI_SOURCES}")
message(STATUS " STMMINPUTGTKXI_EXTRA_INCLUDE_DIRS: ${STMMINPUTGTKXI_EXTRA_INCLUDE_DIRS}")
message(STATUS " STMMINPUTGTKXI_EXTRA_LIBS:         ${STMMINPUTGTKXI_EXTRA_LIBS}")
message(STATUS " CMAKE_BUILD_TYPE:                  ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:             ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:        ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " CMAKE_CXX_FLAGS:                   ${CMAKE_CXX_FLAGS}")
message(STATUS " install prefix:                    ${STMMI_INSTALL_PREFIX}")
message(STATUS " BUILD_DOCS:                        ${BUILD_DOCS}")
message(STATUS " BUILD_TESTING:                     ${BUILD_TESTING}")
if (BUILD_SHARED_LIBS)
message(STATUS " STMMI_PLUGINS_DATA_DIR:            ${STMMI_PLUGINS_DATA_DIR}")
message(STATUS " STMMI_PLUGINS_USER_DATA_DIR:       ${STMMI_PLUGINS_USER_DATA_DIR}")
endif()
endif()

# Documentation
add_subdirectory(doc)
if (BUILD_DOCS) #  OR ("$ENV{STMM_DEBIAN_PACKAGING}" STREQUAL "ON")
    # build automatically, otherwise call `make doc`
    add_dependencies(stmm-input-gtk-xi doc)
endif()

# Testing
enable_testing()
add_subdirectory(test)

install(TARGETS stmm-input-gtk-xi LIBRARY DESTINATION "lib"  ARCHIVE DESTINATION "lib")

install(FILES ${STMMI_HEADERS}   DESTINATION "include/stmm-input-gtk-xi")

install(FILES  "${PROJECT_BINARY_DIR}/stmm-input-gtk-xi.pc"   DESTINATION "lib/pkgconfig")

if (BUILD_SHARED_LIBS)
    install(FILES  "${PROJECT_BINARY_DIR}/${STMMI_PKG_PLUGIN_NAME}.dlp"  DESTINATION "${STMMI_PLUGINS_DATA_DIR}")
endif()

