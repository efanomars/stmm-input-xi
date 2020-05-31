/*
 * Copyright © 2016-2019  Stefano Marsili, <stemars@gmx.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   fakeflogtkbackend.cc
 */

#include "fakeflogtkbackend.h"

#include "fakeflogtkwindowdata.h"

#include <iostream>

namespace stmi
{

namespace testing
{
namespace Flo
{

FakeGtkBackend::FakeGtkBackend(::stmi::FloGtkDeviceManager* p0Owner, const Glib::RefPtr<Gdk::Display>& refGdkDisplay) noexcept
: Private::Flo::GtkBackend(p0Owner)
, m_refGdkDisplay(refGdkDisplay)
, m_nFocusXWinId(None)
{
	const std::string sError = init(refGdkDisplay);
	if (! sError.empty()) {
		std::cerr << "FakeGtkBackend::FakeGtkBackend " << sError<< '\n';
		assert(false);
	}
	assert(refGdkDisplay);
}

void FakeGtkBackend::focusDevicesToWindow(const std::shared_ptr<Private::Flo::GtkWindowData>& refWinData) noexcept
{
//std::cout << "FakeGtkBackend::focusDevicesToWindow" << '\n';
	if (refWinData) {
		m_nFocusXWinId = refWinData->getXWindow();
	} else {
		m_nFocusXWinId = None;
	}
}

void FakeGtkBackend::simulateKeyEvent(int32_t nXDeviceId, ::Window nWinId, bool bPress, int32_t nHardwareCode) noexcept
{
	assert(nWinId != None);
	if (m_nFocusXWinId != nWinId) {
		return;
	}
	#ifndef NDEBUG
	const int32_t nIdx =
	#endif
	findXDeviceId(nXDeviceId);
	assert(nIdx >= 0);

	XIDeviceEvent oXIDeviceEvent;
	oXIDeviceEvent.deviceid = nXDeviceId;
	oXIDeviceEvent.evtype = (bPress ? XI_KeyPress : XI_KeyRelease);
	oXIDeviceEvent.detail = nHardwareCode;
	oXIDeviceEvent.event = nWinId;
	// ALL THE REST OF THE FIELDS IS UNUSED!
	onXIDeviceEvent(&oXIDeviceEvent);
}

} // namespace Flo
} // namespace Private

} // namespace stmi
