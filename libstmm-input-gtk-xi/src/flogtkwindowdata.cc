/*
 * Copyright © 2016-2017  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   flogtkwindowdata.cc
 */

#include "flogtkwindowdata.h"


namespace stmi
{

namespace Private
{
namespace Flo
{

void GtkWindowData::enable(const shared_ptr<GtkAccessor>& refAccessor, FloGtkDeviceManager* p0Owner)
{
	assert(refAccessor);
	assert(p0Owner != nullptr);
	assert(!m_oIsActiveConn.connected());
	m_refAccessor = refAccessor;
	m_p0Owner = p0Owner;
	Gtk::Window* p0GtkmmWindow = m_refAccessor->getGtkmmWindow();
	m_oIsActiveConn = p0GtkmmWindow->property_is_active().signal_changed().connect(sigc::mem_fun(this, &GtkWindowData::onSigIsActiveChanged));
	//
	if (p0GtkmmWindow->get_realized()) {
		setXWinAndXDisplay(p0GtkmmWindow);
	} else {
		// Wait until activation of window to set this field
		m_nXWinId = None;
	}
	m_bIsEnabled = true;
}
void GtkWindowData::setXWinAndXDisplay(Gtk::Window* p0GtkmmWindow)
{
	Glib::RefPtr<Gdk::Window> refWindow = p0GtkmmWindow->get_window();
	GdkWindow* p0GdkWindow = refWindow->gobj();
	m_nXWinId = gdk_x11_window_get_xid(p0GdkWindow);
	assert(m_nXWinId != None);
	//
	m_refGdkDisplay = p0GtkmmWindow->get_display();
	assert(m_refGdkDisplay);
	GdkDisplay* p0GdkDisplay = m_refGdkDisplay->gobj();
	m_p0XDisplay = gdk_x11_display_get_xdisplay(p0GdkDisplay);
	assert(m_p0XDisplay != nullptr);
}
void GtkWindowData::disable()
{
	if (!m_bIsEnabled) {
		return;
	}
	m_bIsEnabled = false;
	m_oIsActiveConn.disconnect();
	disconnectAllDevices();
	m_nXWinId = None;
	m_refGdkDisplay.clear();
	m_p0XDisplay = nullptr;
	m_p0Owner = nullptr;
	// DO NOT CLEAR m_refAccessor because despite being disabled
	// the object's accessor is still used afterwards to
	// send cancels to listeners
}

void GtkWindowData::setXIEventsForAllDevices(bool bSet)
{
	assert(m_p0Owner != nullptr);
	for (auto& oPair : m_p0Owner->m_aKeyboardDevices) {
		const int32_t nDeviceId = oPair.first;
		const bool bConnectedAll = setXIEventsForDevice(nDeviceId, bSet);
		if (bConnectedAll) {
			break; // for -----------
		}
	}
}
bool GtkWindowData::setXIEventsForDevice(int32_t nXDeviceId, bool bSet)
{
//std::cout << "Flo::GtkWindowData::selectXIEvents()  nXDeviceId=" << nXDeviceId << "  nXWinId=" << m_nXWinId << "  bSet=" << bSet << std::endl;
	assert(m_p0Owner != nullptr);
	//assert(m_bIsEnabled);
	if ((!m_refAccessor) || m_refAccessor->isDeleted()) {
		return false; //----------------------------------------------------
	}
	if (!m_bIsRealized) {
		const bool bIsRealized = m_refAccessor->getGtkmmWindow()->get_realized();
		if (!bIsRealized) {
			// the xwindow doesn't even exist yet!
			return false; //------------------------------------------------
		}
		m_bIsRealized = true;
//std::cout << "Flo::GtkWindowData::selectXIEvents()  just realized!" << std::endl;
		setXIEventsForAllDevices(bSet);
		return true; //-----------------------------------------------------
	}
	XIEventMask oEvMasks[ 1 ];

	// TODO To be nice to others selecting events of this device
	// TODO maybe use GetMask | key press + key release
	unsigned char oMask1[ ( XI_LASTEVENT + 7 ) / 8 ];
	memset( oMask1, 0, sizeof( oMask1 ) );

	if (bSet) {
		XISetMask( oMask1, XI_KeyPress );
		XISetMask( oMask1, XI_KeyRelease );
	}
	oEvMasks[ 0 ].deviceid = nXDeviceId; //XIAllDevices;
	oEvMasks[ 0 ].mask_len = sizeof( oMask1 );
	oEvMasks[ 0 ].mask = oMask1;

	assert(m_p0XDisplay != nullptr);
	auto nStatus = XISelectEvents( m_p0XDisplay, m_nXWinId, oEvMasks, sizeof(oEvMasks) / sizeof(oEvMasks[0]));
	if ((nStatus == BadValue) || (nStatus == BadWindow)) {
		std::cout << "FloGtkDeviceManager: XISelectEvents oEvMasks returns ";
		if (nStatus == BadValue) {
			std::cout << "BadValue";
		} else if (nStatus == BadWindow) {
			std::cout << "BadWindow";
		} else {
			assert(false);
		}
		std::cout << std::endl;
		assert(false);
	}
	return false;
}

} // namespace Flo
} // namespace Private

} // namespace stmi
