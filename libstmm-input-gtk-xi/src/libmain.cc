/*
 * Copyright © 2017  Stefano Marsili, <stemars@gmx.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   libmain.cc
 */

#include "libmain.h"

#include "flogtkdevicemanager.h"

#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

shared_ptr<stmi::ChildDeviceManager> createPlugin(const std::string& sAppName
												, bool bEnableEventClasses, const std::vector<stmi::Event::Class>& aEnDisableEventClasses)
{
	shared_ptr<stmi::ChildDeviceManager> refChild; 
	try {
		refChild = stmi::FloGtkDeviceManager::create(sAppName, bEnableEventClasses, aEnDisableEventClasses);
	} catch (const std::runtime_error& oErr) {
		std::cerr << oErr.what() << '\n';
	}
	return refChild;
}

#ifdef __cplusplus
}
#endif
