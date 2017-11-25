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

# File:   googletest_create_links.py

# Use this command to create googletest subdirectories to mimick git submodules


import os
import subprocess

def testDir(sGoogletestSrcDir):
	if not os.path.isdir(sGoogletestSrcDir):
		return False
	sDir = sGoogletestSrcDir + "/include/gtest"
	if (not os.path.exists(sDir)) or not os.path.isdir(sDir):
		return False
	sDir = sGoogletestSrcDir + "/src"
	if (not os.path.exists(sDir)) or not os.path.isdir(sDir):
		return False
	sFile = sGoogletestSrcDir + "/CMakeLists.txt"
	if (not os.path.exists(sFile)) or not os.path.isfile(sFile):
		return False
	return True

def createSymlink(sPrjSubdir, sGoogletestSrcDir):
	os.chdir(sPrjSubdir)
	if os.path.exists("googletest"):
		try:
			os.rmdir("googletest")
		except OSError as oErr:
			raise RuntimeError("Couldn't remove directory " + sPrjSubdir + "/googletest") \
							from oErr
	subprocess.check_call("ln -s {} googletest".format(sGoogletestSrcDir).split())
	os.chdir("..")
	print("created link from '" + sPrjSubdir + "/googletest' to '" + sGoogletestSrcDir + "'")

def main():

	import argparse
	oParser = argparse.ArgumentParser(description="Create googletest subdirectories (symbolic links).\n"
							"Before executing this command make sure the googletest subdirectories\n"
							"are either not present or empty.\n"
							"The default source dir is for the Debian googletest package."
							, formatter_class=argparse.RawDescriptionHelpFormatter)
	oParser.add_argument("--google-src-dir", help="google source dir (default=/usr/src/googletest/googletest, /usr/src/gtest)", metavar='SRCDIR'\
						, default="/usr/src/googletest/googletest", dest="sGoogletestSrcDir")
	oArgs = oParser.parse_args()

	sScriptDir = os.path.dirname(os.path.abspath(__file__))

	os.chdir(sScriptDir)
	os.chdir("..")

	if oArgs.sGoogletestSrcDir == "":
		sSrcDir = "/usr/src/googletest/googletest"
		if not testDir(sSrcDir):
			sSrcDir = "/usr/src/gtest"
			if not testDir(sSrcDir):
				raise RuntimeError("Error: directory containing googletest source code not found!")
	else:
		sSrcDir = os.path.abspath(os.path.expanduser(oArgs.sGoogletestSrcDir))
		if not testDir(sSrcDir):
			raise RuntimeError("Error: '" + sSrcDir + "' not googletest source directory")

	createSymlink("libstmm-input-gtk-xi", sSrcDir)


if __name__ == "__main__":
	main()

