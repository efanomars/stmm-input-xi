# File: device-floater/CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

option(STMM_INSTALL_LAUNCHER "Install launcher in share/applications/ (implies STMM_INSTALL_ICONS=ON)" ON)
option(STMM_INSTALL_ICONS "Install icons in share/icons/hicolor/(size)/apps/" ON)

project(device-floater CXX)

set(RUNTIME_OUTPUT_DIRECTORY "build")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../share/cmake) 

include(CommonUtil)

CheckBinaryNotSourceTree()
CheckBuildType()
DefineCommonCompileOptions()

include(FindPkgConfig)
if (NOT PKG_CONFIG_FOUND)
     message(FATAL_ERROR "Mandatory 'pkg-config' not found!")
endif()

#
set(DEVICE_FLOATER_MAJOR_VERSION 0)
set(DEVICE_FLOATER_MINOR_VERSION 1)
set(DEVICE_FLOATER_VERSION "${DEVICE_FLOATER_MAJOR_VERSION}.${DEVICE_FLOATER_MINOR_VERSION}.0")

set(GTKMM_VERSION_REQ "3.14.0")
set(XI_VERSION_REQ "1.7.4")
set(X11_VERSION_REQ "1.6.2")

pkg_check_modules(GTKMM  REQUIRED  gtkmm-3.0>=${GTKMM_VERSION_REQ})
pkg_check_modules(XI  REQUIRED  xi>=${XI_VERSION_REQ})
pkg_check_modules(X11  REQUIRED  x11>=${X11_VERSION_REQ})

list(APPEND STMMI_DEVFLO_EXTRA_INCLUDE_DIRS   ${GTKMM_INCLUDE_DIRS} ${XI_INCLUDE_DIRS} ${X11_INCLUDE_DIRS})
list(APPEND STMMI_DEVFLO_EXTRA_LIBS           ${GTKMM_LIBRARIES}    ${XI_LIBRARIES}    ${X11_LIBRARIES})

string(STRIP "${STMMI_DEVFLO_EXTRA_LIBS}" STMMI_DEVFLO_EXTRA_LIBS)
string(STRIP "${STMMI_DEVFLO_EXTRA_INCLUDE_DIRS}" STMMI_DEVFLO_EXTRA_INCLUDE_DIRS)

# Source files (and headers only used for building)
set(STMMI_DEVFLO_SOURCES
        ${PROJECT_SOURCE_DIR}/src/config.h
        ${PROJECT_SOURCE_DIR}/src/devicefloater.h
        ${PROJECT_SOURCE_DIR}/src/devicefloater.cc
        ${PROJECT_SOURCE_DIR}/src/main.cc
        ${PROJECT_SOURCE_DIR}/src/xideviceswindow.h
        ${PROJECT_SOURCE_DIR}/src/xideviceswindow.cc
        )
set(STMMI_DEVFLO_DATA_DIR  ${PROJECT_SOURCE_DIR}/data)
set(STMMI_DEVFLO_DATA_FILES
        ${STMMI_DEVFLO_DATA_DIR}/buoy_32313.png
        ${STMMI_DEVFLO_DATA_DIR}/keyboard_1420.png
        ${STMMI_DEVFLO_DATA_DIR}/mouse_347.png
        )

add_executable(device-floater ${STMMI_DEVFLO_SOURCES} ${PROJECT_BINARY_DIR}/config.cc)

target_include_directories(device-floater SYSTEM PUBLIC ${STMMI_DEVFLO_EXTRA_INCLUDE_DIRS})

DefineTargetPublicCompileOptions(device-floater)

include(GNUInstallDirs)
set(STMMI_DEVFLO_PKG_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/device-floater")
set(STMMI_DEVFLO_PKG_REL_DATA_DIR "${CMAKE_INSTALL_DATADIR}/device-floater")

# Create config file for executable
configure_file("${PROJECT_SOURCE_DIR}/src/config.cc.in"
               "${PROJECT_BINARY_DIR}/config.cc" @ONLY)
# This allows config.cc to find the config.h include
target_include_directories(device-floater PUBLIC ${PROJECT_SOURCE_DIR}/src)

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "device-floater was configured with the following options:")
message(STATUS " STMMI_DEVFLO_SOURCES:            ${STMMI_DEVFLO_SOURCES}")
#message(STATUS " STMMI_DEVFLO_EXTRA_INCLUDE_DIRS: ${STMMI_DEVFLO_EXTRA_INCLUDE_DIRS}")
message(STATUS " STMMI_DEVFLO_EXTRA_LIBS:         ${STMMI_DEVFLO_EXTRA_LIBS}")
message(STATUS " STMMI_DEVFLO_DATA_FILES          ${STMMI_DEVFLO_DATA_FILES}")
message(STATUS " CMAKE_BUILD_TYPE:                ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:           ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:      ${CMAKE_CXX_COMPILER_VERSION}")
#message(STATUS " CMAKE_CXX_FLAGS:                 $<TARGET_PROPERTY:device-floater,COMPILE_OPTIONS>")
message(STATUS " install prefix:                  ${CMAKE_INSTALL_PREFIX}")
endif()

target_link_libraries(device-floater ${STMMI_DEVFLO_EXTRA_LIBS}) # ${STMMI_DEVFLO_EXTRA_LDFLAGS}

install(TARGETS device-floater RUNTIME DESTINATION "bin")

install(FILES ${STMMI_DEVFLO_DATA_FILES}  DESTINATION   ${STMMI_DEVFLO_PKG_REL_DATA_DIR})

if (STMM_INSTALL_LAUNCHER)
    install(FILES         "${STMMI_DEVFLO_DATA_DIR}/applications/device-floater.desktop"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/applications/")
endif()
if (STMM_INSTALL_ICONS OR STMM_INSTALL_LAUNCHER)
    install(FILES         "${STMMI_DEVFLO_DATA_DIR}/icons/hicolor/24x24/apps/device-floater.png"
            DESTINATION   "${CMAKE_INSTALL_DATADIR}/icons/hicolor/24x24/apps/")
    install(FILES         "${STMMI_DEVFLO_DATA_DIR}/icons/hicolor/32x32/apps/device-floater.png"
            DESTINATION   "${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps/")
    install(FILES         "${STMMI_DEVFLO_DATA_DIR}/icons/hicolor/48x48/apps/device-floater.png"
            DESTINATION   "${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps/")
    install(FILES         "${STMMI_DEVFLO_DATA_DIR}/icons/hicolor/scalable/apps/device-floater.svg"
            DESTINATION   "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/")
endif()
