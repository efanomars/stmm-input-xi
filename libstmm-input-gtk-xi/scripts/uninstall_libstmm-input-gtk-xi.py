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

# File:   uninstall_libstmm-input-gtk-xi.py

# Removes all files installed by the install_libstmm-input-gtk-xi.py script.

import sys
import os
import subprocess

def main():
	import argparse
	oParser = argparse.ArgumentParser("Removes all files created by install_libstmm-input-gtk-xi.py")
	oParser.add_argument("-r", "--remove-builds", help="Remove build folders", action="store_true"\
						, default=False, dest="bRemoveBuilds")
	oParser.add_argument("-y", "--no-prompt", help="No prompt comfirmation", action="store_true"\
						, default=False, dest="bNoPrompt")
	oParser.add_argument("--destdir", help="install dir (default=/usr/local)", metavar='DESTDIR'\
						, default="/usr/local", dest="sDestDir")
	oParser.add_argument("--no-sudo", help="don't use sudo to uninstall", action="store_true"\
						, default=False, dest="bDontSudo")
	oArgs = oParser.parse_args()

	sDestDir = os.path.abspath(oArgs.sDestDir)

	sScriptDir = os.path.dirname(os.path.abspath(__file__))
	os.chdir(sScriptDir)
	os.chdir("..")

	if not oArgs.bNoPrompt:
		print("Uninstall from dir: " + sDestDir + "   Remove build folders: " + str(oArgs.bRemoveBuilds))

	while not oArgs.bNoPrompt:
		sAnswer = input("Are you sure? (yes/no) >").strip()
		if sAnswer.casefold() == "yes":
			break
		elif sAnswer.casefold() == "no":
			return #-----------------------------------------------------------

	if oArgs.bDontSudo:
		sSudo = ""
	else:
		sSudo = "sudo"

	subprocess.check_call("{} rm -r -f             {}/include/stmm-input-gtk-xi".format(sSudo, sDestDir).split())
	subprocess.check_call("{} rm    -f              {}/lib/libstmm-input-gtk-xi.so*".format(sSudo, sDestDir), shell=True)
	subprocess.check_call("{} rm    -f              {}/lib/libstmm-input-gtk-xi.a".format(sSudo, sDestDir).split())
	subprocess.check_call("{} rm    -f       {}/lib/pkgconfig/stmm-input-gtk-xi.pc".format(sSudo, sDestDir).split())
	subprocess.check_call("{} rm -r -f           {}/share/doc/stmm-input-gtk-xi".format(sSudo, sDestDir).split())
	subprocess.check_call("{} rm    -f {}/share/stmm-input-dl/stmm-input-gtk-xi.dlp".format(sSudo, sDestDir).split())

	if oArgs.bRemoveBuilds:
		os.chdir("..")
		subprocess.check_call("{} rm -r -f libstmm-input-gtk-xi/build".format(sSudo).split())

	if not oArgs.bDontSudo:
		subprocess.check_call("sudo ldconfig".split())


if __name__ == "__main__":
	main()

