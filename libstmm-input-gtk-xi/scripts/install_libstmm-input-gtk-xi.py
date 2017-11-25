#!/usr/bin/env python3

#  Copyright © 2017  Stefano Marsili, <stemars@gmx.ch>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, see <http://www.gnu.org/licenses/>

# File:   install_libstmm-input-gtk-xi.py

# Compiles and installs the libstmm-input-gtk-xi library.

import sys
import os
import subprocess

def main():
	import argparse
	oParser = argparse.ArgumentParser()
	oParser.add_argument("-s", "--staticlib", help="build static library (instead of shared)", choices=['On', 'Off', 'Cache']\
						, default="Cache", dest="sBuildStaticLib")
	oParser.add_argument("-b", "--buildtype", help="build type (default=Release)"\
						, choices=['Debug', 'Release', 'MinSizeRel', 'RelWithDebInfo']\
						, default="Release", dest="sBuildType")
	oParser.add_argument("-t", "--tests", help="build tests (default=Cache)", choices=['On', 'Off', 'Cache']\
						, default="Cache", dest="sBuildTests")
	oParser.add_argument("-d", "--docs", help="build documentation (default=Cache)", choices=['On', 'Off', 'Cache']\
						, default="Cache", dest="sBuildDocs")
	oParser.add_argument("--docs-to-log", help="--docs warnings to log file", action="store_true"\
						, default=False, dest="bDocsWarningsToLog")
	oParser.add_argument("--destdir", help="install dir (default=/usr/local)", metavar='DESTDIR'\
						, default="/usr/local", dest="sDestDir")
	oParser.add_argument("--no-sudo", help="don't use sudo to install", action="store_true"\
						, default=False, dest="bDontSudo")
	oParser.add_argument("--sanitize", help="compile with -fsanitize=address (Debug only)", action="store_true"\
						, default=False, dest="bSanitize")
	oArgs = oParser.parse_args()

	sDestDir = os.path.abspath(oArgs.sDestDir)

	sScriptDir = os.path.dirname(os.path.abspath(__file__))
	#print("sScriptDir:" + sScriptDir)
	os.chdir(sScriptDir)
	os.chdir("..")
	#
	sBuildSharedLib = "-D BUILD_SHARED_LIBS="
	if oArgs.sBuildStaticLib == "On":
		sBuildSharedLib += "OFF"
	elif oArgs.sBuildStaticLib == "Off":
		sBuildSharedLib += "ON"
	else:
		sBuildSharedLib = ""
	#print("sBuildSharedLib:" + sBuildSharedLib)
	#
	sBuildTests = "-D BUILD_TESTING="
	if oArgs.sBuildTests == "On":
		sBuildTests += "ON"
	elif oArgs.sBuildTests == "Off":
		sBuildTests += "OFF"
	else:
		sBuildTests = ""
	#print("sBuildTests:" + sBuildTests)
	#
	sBuildDocs = "-D BUILD_DOCS="
	if oArgs.sBuildDocs == "On":
		sBuildDocs += "ON"
	elif oArgs.sBuildDocs == "Off":
		sBuildDocs += "OFF"
	else:
		sBuildDocs = ""
	#print("sBuildDocs:" + sBuildDocs)
	#
	sDocsWarningsToLog = "-D BUILD_DOCS_WARNINGS_TO_LOG_FILE="
	if oArgs.bDocsWarningsToLog:
		sDocsWarningsToLog += "ON"
	else:
		sDocsWarningsToLog += "OFF"
	#print("sDocsWarningsToLog:" + sDocsWarningsToLog)
	#
	sDestDir = "-D CMAKE_INSTALL_PREFIX=" + sDestDir
	#print("sDestDir:" + sDestDir)
	#
	sBuildType = "-D CMAKE_BUILD_TYPE=" + oArgs.sBuildType
	#print("sBuildType:" + sBuildType)
	#
	if oArgs.bSanitize:
		sSanitize = "-D BUILD_WITH_SANITIZE=ON"
	else:
		sSanitize = ""
	#print("sSanitize:" + sSanitize)

	#
	if oArgs.bDontSudo:
		sSudo = ""
	else:
		sSudo = "sudo"

	#
	if not os.path.isdir("build"):
		os.mkdir("build")

	os.chdir("build")

	subprocess.check_call("cmake {} {} {} {} {} {} {} ..".format(\
			sBuildSharedLib, sBuildTests, sBuildDocs, sDocsWarningsToLog, sBuildType\
			, sDestDir, sSanitize).split())
	subprocess.check_call("make $STMM_MAKE_OPTIONS", shell=True)
	subprocess.check_call("{} make install".format(sSudo).split())

	if oArgs.sBuildStaticLib == "Cache":
		oProc = subprocess.Popen("cmake -N -LA".split(), stdout=subprocess.PIPE, shell=False)
		(sOut, sErr) = oProc.communicate()
		bSharedLib = ("BUILD_SHARED_LIBS:BOOL=ON" in str(sOut))
	else:
		bSharedLib = (oArgs.sBuildStaticLib == "Off")

	if bSharedLib and not oArgs.bDontSudo:
		subprocess.check_call("sudo ldconfig".split())


if __name__ == "__main__":
	main()

