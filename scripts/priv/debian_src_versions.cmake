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

# File:   debian_src_versions.cmake


# This has to be run with
#   cmake -DSTMMI_PROJECT_SOURCE_DIR="/home/.../libstmm-input-gtk-xi" ... -P debian_src_versions.cmake

# Parameters:
#   STMMI_DEBIAN_SRC_FILENAME     The template file name in ${STMMI_DEBIAN_SRC_FROM_DIR}
#                                 without the ".in" suffix.
#                                 (ex. if value is "control" the file name is "control.in")
#                                 (ex. "libstmm-input-gtk-xi@STMM_INPUT_GTK_XI_MAJOR_VERSION@.install"
#   STMMI_DEBIAN_SRC_FROM_DIR     The absolute path to the directory containing file
#                                 "${STMMI_DEBIAN_SRC_FILENAME}.in"
#                                 (ex. .../stmm-input-xi/scripts/priv/debian.orig)
#   STMMI_DEBIAN_SRC_TO_DIR       The absolute path to the target directory
#                                 (ex. /tmp/stmm-input-xiDeb/stmm-input-xi-0.1/debian)
#   STMMI_DEBIAN_SRC_TMP_1        The absolute path to a unique temporary file used by this function.
#                                 If empty don't configure file name itself.
#   STMMI_DEBIAN_SRC_TMP_2        The absolute path to a another unique temporary file used by this function.
#   STMMI_PROJECT_SOURCE_DIR      The absolute path to one of the subproject directories

set(PROJECT_SOURCE_DIR "${STMMI_PROJECT_SOURCE_DIR}")

include("${PROJECT_SOURCE_DIR}/../libstmm-input-gtk-xi/stmm-input-gtk-xi-defs.cmake")
include("${PROJECT_SOURCE_DIR}/../device-floater/device-floater-defs.cmake")

if ("${STMMI_DEBIAN_SRC_TMP_1}" STREQUAL "")
    set(STMMI_DEBIAN_SRC_RESNAME "${STMMI_DEBIAN_SRC_FILENAME}")
else()
    # write the template file name to the temporary
    file(WRITE "${STMMI_DEBIAN_SRC_TMP_1}" "${STMMI_DEBIAN_SRC_FILENAME}")
    # sobstitute version numbers if file name contains any
    configure_file("${STMMI_DEBIAN_SRC_TMP_1}" "${STMMI_DEBIAN_SRC_TMP_2}" @ONLY)
    # read the destination file name from new temporary
    file(STRINGS "${STMMI_DEBIAN_SRC_TMP_2}" STMMI_DEBIAN_SRC_RESNAME)
endif()

configure_file("${STMMI_DEBIAN_SRC_FROM_DIR}/${STMMI_DEBIAN_SRC_FILENAME}.in"
               "${STMMI_DEBIAN_SRC_TO_DIR}/${STMMI_DEBIAN_SRC_RESNAME}" @ONLY)
