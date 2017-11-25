# File: libstmm-input-gtk-xi/CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

project(stmm-input-gtk-xi CXX)

set(LIBRARY_OUTPUT_DIRECTORY "build")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../share/cmake) 

include(CommonUtil)

CheckBinaryNotSourceTree()
CheckBuildType()
DefineSharedLibOption()
DefineCommonOptions()
DefineCommonCompileOptions()

include(FindPkgConfig)
if (NOT PKG_CONFIG_FOUND)
     message(FATAL_ERROR "Mandatory 'pkg-config' not found!")
endif()

# Libtool CURRENT/REVISION/AGE: here
#   MAJOR is CURRENT interface
#   MINOR is REVISION (implementation of interface)
#   AGE is always 0
set(STMM_INPUT_GTK_XI_MAJOR_VERSION 0)
set(STMM_INPUT_GTK_XI_MINOR_VERSION 1)
set(STMM_INPUT_GTK_XI_VERSION "${STMM_INPUT_GTK_XI_MAJOR_VERSION}.${STMM_INPUT_GTK_XI_MINOR_VERSION}.0")

# required stmm-input-ev version
set(STMM_INPUT_EV_MAJOR_VERSION_REQ 0)
set(STMM_INPUT_EV_MINOR_VERSION_REQ 1)
set(STMM_INPUT_EV_VERSION_REQ "${STMM_INPUT_EV_MAJOR_VERSION_REQ}.${STMM_INPUT_EV_MINOR_VERSION_REQ}")

# required stmm-input-ev version
set(STMM_INPUT_GTK_MAJOR_VERSION_REQ 0)
set(STMM_INPUT_GTK_MINOR_VERSION_REQ 1)
set(STMM_INPUT_GTK_VERSION_REQ "${STMM_INPUT_GTK_MAJOR_VERSION_REQ}.${STMM_INPUT_GTK_MINOR_VERSION_REQ}")

set(XI_VERSION_REQ "1.7.4")
set(X11_VERSION_REQ "1.6.2")

# Beware! The prefix passed to pkg_check_modules(PREFIX ...) shouldn't contain underscores!
pkg_check_modules(STMMINPUTEV    REQUIRED  stmm-input-ev>=${STMM_INPUT_EV_VERSION_REQ})
pkg_check_modules(STMMINPUTGTK   REQUIRED  stmm-input-gtk>=${STMM_INPUT_GTK_VERSION_REQ})
pkg_check_modules(XI             REQUIRED  xi>=${XI_VERSION_REQ})
pkg_check_modules(X11            REQUIRED  x11>=${X11_VERSION_REQ})

list(APPEND STMMI_GTK_XI_EXTRA_INCLUDE_DIRS
    ${STMMINPUTEV_INCLUDE_DIRS}
    ${STMMINPUTGTK_INCLUDE_DIRS}
    ${XI_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
    )
list(APPEND STMMI_GTK_XI_EXTRA_LIBS
    ${STMMINPUTGTK_LIBRARIES}
    ${STMMINPUTEV_LIBRARIES}
    ${XI_LIBRARIES}
    ${X11_LIBRARIES}
    )

string(STRIP "${STMMI_GTK_XI_EXTRA_LIBS}"         STMMI_GTK_XI_EXTRA_LIBS)
string(STRIP "${STMMI_GTK_XI_EXTRA_INCLUDE_DIRS}" STMMI_GTK_XI_EXTRA_INCLUDE_DIRS)

# Headers dir
set(STMMI_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}/include")
set(STMMI_HEADERS_DIR  "${STMMI_INCLUDE_DIR}/stmm-input-gtk-xi")

set(STMMI_HEADERS
        ${STMMI_HEADERS_DIR}/flogtkdevicemanager.h
        ${STMMI_HEADERS_DIR}/stmm-input-gtk-xi.h
        ${STMMI_HEADERS_DIR}/stmm-input-gtk-xi-config.h
        )
if (BUILD_SHARED_LIBS)
    set(STMMI_HEADERS  ${STMMI_HEADERS}
        ${STMMI_HEADERS_DIR}/libmain.h
        )
endif()
#
# Sources dir
set(STMMI_SOURCES_DIR  "${PROJECT_SOURCE_DIR}/src")
# Source files (and headers only used for building)
set(STMMI_SOURCES
        ${STMMI_SOURCES_DIR}/floatingsources.h
        ${STMMI_SOURCES_DIR}/floatingsources.cc
        ${STMMI_SOURCES_DIR}/flogtkbackend.h
        ${STMMI_SOURCES_DIR}/flogtkbackend.cc
        ${STMMI_SOURCES_DIR}/flogtkdevicemanager.cc
        ${STMMI_SOURCES_DIR}/flogtklistenerextradata.h
        ${STMMI_SOURCES_DIR}/flogtkwindowdata.h
        ${STMMI_SOURCES_DIR}/flogtkwindowdata.cc
        ${STMMI_SOURCES_DIR}/flogtkxkeyboarddevice.h
        ${STMMI_SOURCES_DIR}/flogtkxkeyboarddevice.cc
        ${STMMI_SOURCES_DIR}/recycler.h
        )
if (BUILD_SHARED_LIBS)
    set(STMMI_SOURCES  ${STMMI_SOURCES}
        ${STMMI_SOURCES_DIR}/libmain.cc
        )
endif()

# Define library
add_library(stmm-input-gtk-xi ${STMMI_SOURCES} ${PROJECT_BINARY_DIR}/stmm-input-gtk-xi-config.cc)

include_directories(SYSTEM "${STMMI_GTK_XI_EXTRA_INCLUDE_DIRS}")

DefineTargetPublicCompileOptions(stmm-input-gtk-xi)

# Set version for stmm-input-gtk-xi-config.cc.in
set(STMMI_PKG_VERSION "${STMM_INPUT_GTK_XI_VERSION}")
set(STMMI_PKG_PLUGIN_NAME "stmm-input-gtk-xi")
# Create config file for library
configure_file("${PROJECT_SOURCE_DIR}/src/stmm-input-gtk-xi-config.cc.in"
               "${PROJECT_BINARY_DIR}/stmm-input-gtk-xi-config.cc" @ONLY)

# Set install dir for pkg-config file
set(STMMI_GTK_XI_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
# Create pkg-config file for shared library
configure_file("${PROJECT_SOURCE_DIR}/stmm-input-gtk-xi.pc.in"
               "${PROJECT_BINARY_DIR}/stmm-input-gtk-xi.pc" @ONLY)

include(GNUInstallDirs)
set(STMMI_PLUGINS_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/stmm-input-dl")
set(STMMI_PLUGINS_USER_DATA_DIR "$ENV{HOME}/.local/${CMAKE_INSTALL_DATADIR}/stmm-input-dl")

set_target_properties(stmm-input-gtk-xi PROPERTIES  ${STMMI_BUILD_TYPE}_POSTFIX "")

set(STMMI_GTK_XI_INSTALL_LIB_PATH1 "${CMAKE_INSTALL_PREFIX}/lib/libstmm-input-gtk-xi.so")
set(STMMI_GTK_XI_INSTALL_LIB_PATH2 "${CMAKE_INSTALL_PREFIX}/lib/libstmm-input-gtk-xi.so.${STMM_INPUT_GTK_XI_MAJOR_VERSION}")
set(STMMI_GTK_XI_INSTALL_LIB_PATH3 "libstmm-input-gtk-xi.so")
set(STMMI_GTK_XI_INSTALL_LIB_PATH4 "libstmm-input-gtk-xi-so.${STMM_INPUT_GTK_XI_MAJOR_VERSION}")
configure_file("${PROJECT_SOURCE_DIR}/${STMMI_PKG_PLUGIN_NAME}.dlp.in"
               "${PROJECT_BINARY_DIR}/${STMMI_PKG_PLUGIN_NAME}.dlp" @ONLY)

# TODO only if plugin => create option COMPILE_AS_PLUGIN=ON (default)
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") AND (BUILD_SHARED_LIBS))
    target_compile_options(stmm-input-gtk-xi PUBLIC "-Wl,-E")
endif()

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "stmm-input-gtk-xi was configured with the following options:")
message(STATUS " STMMI_HEADERS:                   ${STMMI_HEADERS}")
message(STATUS " STMMI_SOURCES:                   ${STMMI_SOURCES}")
message(STATUS " STMMI_GTK_XI_EXTRA_INCLUDE_DIRS: ${STMMI_GTK_XI_EXTRA_INCLUDE_DIRS}")
message(STATUS " STMMI_GTK_XI_EXTRA_LIBS:         ${STMMI_GTK_XI_EXTRA_LIBS}")
message(STATUS " CMAKE_BUILD_TYPE:                ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:           ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:      ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " CMAKE_CXX_FLAGS:                 ${CMAKE_CXX_FLAGS}")
message(STATUS " install prefix:                  ${STMMI_GTK_XI_INSTALL_PREFIX}")
message(STATUS " BUILD_DOCS:                      ${BUILD_DOCS}")
message(STATUS " BUILD_TESTING:                   ${BUILD_TESTING}")
if (BUILD_SHARED_LIBS)
message(STATUS " STMMI_PLUGINS_DATA_DIR:          ${STMMI_PLUGINS_DATA_DIR}")
message(STATUS " STMMI_PLUGINS_USER_DATA_DIR:     ${STMMI_PLUGINS_USER_DATA_DIR}")
endif()
endif()

# Documentation
add_subdirectory(doc)
if (BUILD_DOCS)
    # build automatically, otherwise call `make doc`
    add_dependencies(stmm-input-gtk-xi doc)
endif()

# Testing
add_subdirectory(googletest)
enable_testing()
add_subdirectory(test)

# Set library versions
set_target_properties(stmm-input-gtk-xi PROPERTIES
        VERSION ${STMM_INPUT_GTK_XI_VERSION}
        SOVERSION ${STMM_INPUT_GTK_XI_MAJOR_VERSION}
#        OUTPUT_NAME "stmm-input-gtk-xi-${STMM_INPUT_GTK_XI_VERSION}"
        )

# Determine header installation dir
target_include_directories(stmm-input-gtk-xi    BEFORE PUBLIC
  $<BUILD_INTERFACE:${STMMI_HEADERS_DIR}>
  $<INSTALL_INTERFACE:include/stmm-input-gtk-xi>  # <prefix>/include/stmm-input-gtk-xi
)

target_link_libraries(stmm-input-gtk-xi ${STMMI_GTK_XI_EXTRA_LIBS}) # ${STMMI_GTK_XI_EXTRA_LDFLAGS}

install(TARGETS stmm-input-gtk-xi LIBRARY DESTINATION "lib"  ARCHIVE DESTINATION "lib")

install(FILES ${STMMI_HEADERS}   DESTINATION "include/stmm-input-gtk-xi")

install(FILES  ${PROJECT_BINARY_DIR}/stmm-input-gtk-xi.pc   DESTINATION "lib/pkgconfig")

if (BUILD_SHARED_LIBS)
    install(FILES  ${PROJECT_BINARY_DIR}/stmm-input-gtk-xi.dlp  DESTINATION "${STMMI_PLUGINS_DATA_DIR}")
endif()

