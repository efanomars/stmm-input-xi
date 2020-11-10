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

# File:   device-floater-defs.cmake

# Libtool CURRENT/REVISION/AGE: here
#   MAJOR is CURRENT interface
#   MINOR is REVISION (implementation of interface)
#   AGE is always 0
set(DEVICE_FLOATER_MAJOR_VERSION 0)
set(DEVICE_FLOATER_MINOR_VERSION 17) # !-U-!
set(DEVICE_FLOATER_VERSION "${DEVICE_FLOATER_MAJOR_VERSION}.${DEVICE_FLOATER_MINOR_VERSION}.0")

set(DEVICE_FLOATER_REQ_GTKMM_VERSION "3.22.0")
set(DEVICE_FLOATER_REQ_XI_VERSION    "1.7.4")
set(DEVICE_FLOATER_REQ_X11_VERSION   "1.6.2")

if ("${CMAKE_SCRIPT_MODE_FILE}" STREQUAL "")
    include(FindPkgConfig)
    if (NOT PKG_CONFIG_FOUND)
        message(FATAL_ERROR "Mandatory 'pkg-config' not found!")
    endif()
    # Beware! The prefix passed to pkg_check_modules(PREFIX ...) shouldn't contain underscores!
    pkg_check_modules(GTKMM       REQUIRED  gtkmm-3.0>=${DEVICE_FLOATER_REQ_GTKMM_VERSION})
    pkg_check_modules(XI          REQUIRED  xi>=${DEVICE_FLOATER_REQ_XI_VERSION})
    pkg_check_modules(X11         REQUIRED  x11>=${DEVICE_FLOATER_REQ_X11_VERSION})
endif()

# include dirs
list(APPEND DEVICEFLOATER_EXTRA_INCLUDE_DIRS  "${GTKMM_INCLUDE_DIRS}")
list(APPEND DEVICEFLOATER_EXTRA_INCLUDE_DIRS  "${XI_INCLUDE_DIRS}")
list(APPEND DEVICEFLOATER_EXTRA_INCLUDE_DIRS  "${X11_INCLUDE_DIRS}")

# libs
list(APPEND DEVICEFLOATER_EXTRA_LIBRARIES     "${GTKMM_LIBRARIES}")
list(APPEND DEVICEFLOATER_EXTRA_LIBRARIES     "${XI_LIBRARIES}")
list(APPEND DEVICEFLOATER_EXTRA_LIBRARIES     "${X11_LIBRARIES}")
