/*
 * Copyright © 2016-2020  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   floatingsources.cc
 */

#include "floatingsources.h"

//#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <cassert>
#ifndef NDEBUG
//#include <iostream>
#endif //NDEBUG

#include <X11/X.h>
#include <X11/extensions/XI2.h>
#include <stdint.h>

namespace stmi
{

namespace Private
{
namespace Flo
{

XIEventSource::XIEventSource(Gdk::Display* p0Display, int nXiOpcode) noexcept
: Glib::Source()
, m_p0XDisplay(None)
, m_nXiOpcode(nXiOpcode)
, m_bSkipNextEvent(false)
{
	static_assert(sizeof(int32_t) >= sizeof(int), "");
	assert(TRUE == true);
	assert(FALSE == false);
	assert(p0Display != nullptr);
	m_p0XDisplay = ::gdk_x11_display_get_xdisplay(p0Display->gobj());
	int nX11ConnectionNumber = ConnectionNumber(m_p0XDisplay);
	m_oPollFd.set_fd(nX11ConnectionNumber);
	m_oPollFd.set_events(Glib::IO_IN);
	add_poll(m_oPollFd);
	// set priority (one) higher than default
	// The default priority is assumed to be the one used by the Gtk main event handler.
	// This allows to fish out the events for floating devices before they get
	// extracted from the queue (and ignored) by the Gtk main event handler.
	set_priority( (Glib::PRIORITY_DEFAULT > Glib::PRIORITY_LOW) ? Glib::PRIORITY_DEFAULT + 1 : Glib::PRIORITY_DEFAULT - 1);
	set_can_recurse(false);
}
XIEventSource::~XIEventSource() noexcept
{
//std::cout << "XIEventSource::~XIEventSource()" << std::endl;
}
sigc::connection XIEventSource::connect(const sigc::slot<bool, ::XIDeviceEvent*>& oSlot) noexcept
{
	return connect_generic(oSlot);
}

bool XIEventSource::prepare(int& nTimeout) noexcept
{
	if (m_bSkipNextEvent) {
		// Since there's an event that wasn't dispatched, no wait
		nTimeout = 0;
	} else {
		// poll can block
		nTimeout = -1;
	}
	// never ready before poll
	return false;
}
bool XIEventSource::checkEvent(::XEvent& oXEvent, bool& bIsGenericEvent, bool& bIsForFloating, bool& bIsFloatingEnterFocus) noexcept
{
	bool bIsFloatingKeyEvent = false;
	bIsForFloating = false;
	bIsFloatingEnterFocus = false;
	bIsGenericEvent = (oXEvent.type == GenericEvent);
	if (bIsGenericEvent) {
		::XGenericEventCookie* p0Cookie = &(oXEvent.xcookie);
		if (::XGetEventData(m_p0XDisplay, p0Cookie)) {
			if (p0Cookie->extension != m_nXiOpcode) {
				bIsGenericEvent = false;
			} else {
				::XIEvent* p0XIEv = reinterpret_cast<::XIEvent *>(p0Cookie->data);
				const auto nEvType = p0Cookie->evtype;
				switch (nEvType) {
				case XI_KeyPress:
				case XI_KeyRelease:
				{
					::XIDeviceEvent* p0XIDeviceEv = reinterpret_cast<::XIDeviceEvent *>(p0XIEv);
					const int nDeviceId = p0XIDeviceEv->deviceid;
					if (nDeviceId == p0XIDeviceEv->sourceid) {
						// It's either a floating or slave device
						int nDevices = 0;
						::XIDeviceInfo* pDevice = ::XIQueryDevice(m_p0XDisplay, nDeviceId, &nDevices);
						if (pDevice->use == XIFloatingSlave) {
							assert(nDevices == 1);
							bIsForFloating = true;
							bIsFloatingKeyEvent = true;
						}
						::XIFreeDeviceInfo(pDevice);
					}
				} break;
				case XI_Enter:
				case XI_Leave:
				case XI_FocusIn:
				case XI_FocusOut:
				{
					::XIEnterEvent* p0XIEnterEv = reinterpret_cast<::XIEnterEvent *>(p0XIEv);
					const int nDeviceId = p0XIEnterEv->deviceid;
					if (nDeviceId == p0XIEnterEv->sourceid) {
						int nDevices = 0;
						::XIDeviceInfo* pDevice = ::XIQueryDevice(m_p0XDisplay, nDeviceId, &nDevices);
						if (pDevice->use == XIFloatingSlave) {
							assert(nDevices == 1);
							bIsForFloating = true;
							bIsFloatingEnterFocus = true;
						}
						::XIFreeDeviceInfo(pDevice);
					}
				} break;
				}
			}
			::XFreeEventData(m_p0XDisplay, p0Cookie);
		}
	}
	return bIsFloatingKeyEvent;
}
bool XIEventSource::check() noexcept
{
	if (((m_oPollFd.get_revents() & Glib::IO_IN) == 0) || (::XPending(m_p0XDisplay) <= 0)) {
		// There is nothing in the queue!
		m_bSkipNextEvent = false;
		return false; //--------------------------------------------------------
	}
	if (m_bSkipNextEvent) {
		m_bSkipNextEvent = false;
		::XEvent oXEvent;
		::XPeekEvent(m_p0XDisplay, &oXEvent);
		bool bIsGenericEvent;
		bool bIsForFloating;
		bool bIsFloatingEnterFocus;
		const bool bIsFloatingKey = checkEvent(oXEvent, bIsGenericEvent, bIsForFloating, bIsFloatingEnterFocus);
		if (bIsFloatingKey) {
			// It's one of my events, cancel the skip
			return true; //-----------------------------------------------------
		}
		if (!bIsForFloating) {
			// It's not even for a floating device, leave it to Gdk::DeviceManager
			return false; //----------------------------------------------------
		}
		if (bIsFloatingEnterFocus) {
			// Since Gdk::DeviceManager doesn't check the device type
			// need to remove the event from the queue in dispatch()
			// before it gets to the Gdk::DeviceManager.
			// These events are generated by XISetFocus in FloGtkDeviceManager
			return true; //-----------------------------------------------------
		}
		// Let the Gdk::DeviceManager remove the event
		return false; //--------------------------------------------------------
	}
	// Since between check and dispatch sometimes events get added to the queue
	// go to dispatch and check there
	return true;
}
bool XIEventSource::dispatch(sigc::slot_base* p0Slot) noexcept
{
	bool bContinue = true;

	if (::XPending(m_p0XDisplay) <= 0) {
		// Some higher priority event removed the event
		m_bSkipNextEvent = false;
		return bContinue;
	}
	::XEvent oXEvent;
	::XPeekEvent(m_p0XDisplay, &oXEvent);
	bool bIsGenericEvent;
	bool bIsForFloating;
	bool bIsFloatingEnterFocus;
	const bool bIsFloatingKey = checkEvent(oXEvent, bIsGenericEvent, bIsForFloating, bIsFloatingEnterFocus);
	if (!bIsFloatingKey) {
		if (!bIsForFloating) {
			// An event for the Gdk::DeviceManager
			m_bSkipNextEvent = true;
			return bContinue; //------------------------------------------------
		}
		// Just remove the event from the queue
		::XNextEvent(m_p0XDisplay, &oXEvent);
		m_bSkipNextEvent = false;
		return bContinue; //----------------------------------------------------
	}
	::XNextEvent(m_p0XDisplay, &oXEvent);
	assert(oXEvent.type == GenericEvent);
	::XGenericEventCookie* p0Cookie = &(oXEvent.xcookie);
	{
		#ifndef NDEBUG
		const bool bRet =
		#endif //NDEBUG
		::XGetEventData(m_p0XDisplay, p0Cookie);
		assert(bRet);
		assert(p0Cookie->extension == m_nXiOpcode);
		assert((p0Cookie->evtype == XI_KeyPress) || (p0Cookie->evtype == XI_KeyRelease));
		::XIEvent* p0XIEv = reinterpret_cast<::XIEvent *>(p0Cookie->data);
		::XIDeviceEvent* p0XIDeviceEv = reinterpret_cast<::XIDeviceEvent*>(p0XIEv);
		const int nDeviceId = p0XIDeviceEv->deviceid;
		assert(nDeviceId == p0XIDeviceEv->sourceid);
		int nDevices = 0;
		{
			::XIDeviceInfo* pDevice = ::XIQueryDevice(m_p0XDisplay, nDeviceId, &nDevices);
			assert(pDevice->use == XIFloatingSlave);
			assert(nDevices == 1);
			bContinue = (*static_cast<sigc::slot<bool, ::XIDeviceEvent*>*>(p0Slot))(p0XIDeviceEv);
			::XIFreeDeviceInfo(pDevice);
		}
		::XFreeEventData(m_p0XDisplay, p0Cookie);
	}
	m_bSkipNextEvent = false;
	return bContinue;
}

} // namespace Flo
} // namespace Private

} // namespace stmi
