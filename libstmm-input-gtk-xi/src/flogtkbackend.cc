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
 * File:   flogtkbackend.cc
 */

#include "flogtkbackend.h"

#include "flogtkwindowdata.h"

#include "flogtkdevicemanager.h"
//#include "floatingsources.h"

#include <stmm-input-gtk/gtkaccessor.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkwindow.h>

#include <cassert>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iterator>

namespace stmi
{

namespace Private
{
namespace Flo
{

void gdkDeviceManagerCallbackAdded(::GdkDeviceManager* p0DeviceManager, ::GdkDevice* p0Device, ::gpointer p0Data) noexcept
{
	auto p0GtkBackend = static_cast<GtkBackend*>(p0Data);
	p0GtkBackend->gdkDeviceAdded(p0DeviceManager, p0Device);
}
void gdkDeviceManagerCallbackChanged(::GdkDeviceManager* p0DeviceManager, ::GdkDevice* p0Device, ::gpointer p0Data) noexcept
{
	auto p0GtkBackend = static_cast<GtkBackend*>(p0Data);
	p0GtkBackend->gdkDeviceChanged(p0DeviceManager, p0Device);
}
void gdkDeviceManagerCallbackRemoved(::GdkDeviceManager* p0DeviceManager, ::GdkDevice* p0Device, ::gpointer p0Data) noexcept
{
	auto p0GtkBackend = static_cast<GtkBackend*>(p0Data);
	p0GtkBackend->gdkDeviceRemoved(p0DeviceManager, p0Device);
}

::GdkFilterReturn gdkWindowEventFilter(::GdkXEvent* p0XEevent, ::GdkEvent* /*p0Event*/, ::gpointer p0Data) noexcept
{
	GtkBackend* p0GtkBackend = static_cast<GtkBackend*>(p0Data);
	XEvent* p0XEv = static_cast<XEvent*>(p0XEevent);

	if (p0XEv->type != GenericEvent) {
		return ::GDK_FILTER_CONTINUE; //----------------------------------------
	}
	::XGenericEventCookie* p0Cookie = &(p0XEv->xcookie);
	if (p0Cookie->extension != p0GtkBackend->m_nXIOpcode) {
		return ::GDK_FILTER_CONTINUE; //----------------------------------------
	}
	::XIEvent* p0XIEv = reinterpret_cast<::XIEvent *>(p0Cookie->data);
	const auto nEvType = p0Cookie->evtype;
	switch (nEvType) {
	case XI_KeyPress:
	case XI_KeyRelease:
	{
		::XIDeviceEvent* p0XIDeviceEv = reinterpret_cast<::XIDeviceEvent *>(p0XIEv);
		const int nDeviceId = p0XIDeviceEv->deviceid;
		if (nDeviceId != p0XIDeviceEv->sourceid) {
			return ::GDK_FILTER_CONTINUE; //------------------------------------
		}
		// It's either a floating or slave device
		int nDevices = 0;
		::XIDeviceInfo* pDevice = ::XIQueryDevice(p0GtkBackend->m_p0XDisplay, nDeviceId, &nDevices);
		const bool bIsFloating = (pDevice->use == XIFloatingSlave);
		if (bIsFloating) {
			assert(nDevices == 1);
			//
			p0GtkBackend->doXIDeviceEventCallback(p0XIDeviceEv);
		}
		::XIFreeDeviceInfo(pDevice);
		if (! bIsFloating) {
			return ::GDK_FILTER_CONTINUE; //------------------------------------
		}
		//
	} break;
	case XI_Enter:
	case XI_Leave:
	case XI_FocusIn:
	case XI_FocusOut:
	{
		::XIEnterEvent* p0XIEnterEv = reinterpret_cast<::XIEnterEvent *>(p0XIEv);
		const int nDeviceId = p0XIEnterEv->deviceid;
		if (nDeviceId != p0XIEnterEv->sourceid) {
			return ::GDK_FILTER_CONTINUE; //------------------------------------
		}
		// It's either a floating or slave device
		int nDevices = 0;
		::XIDeviceInfo* pDevice = ::XIQueryDevice(p0GtkBackend->m_p0XDisplay, nDeviceId, &nDevices);
		const bool bIsFloating = (pDevice->use == XIFloatingSlave);
		if (bIsFloating) {
			assert(nDevices == 1);
		}
		::XIFreeDeviceInfo(pDevice);
		if (! bIsFloating) {
			return ::GDK_FILTER_CONTINUE; //------------------------------------
		}
	} break;
	default:
	{
		return ::GDK_FILTER_CONTINUE; //----------------------------------------
	}
	}
	return ::GDK_FILTER_REMOVE;
}

GtkBackend::GtkBackend(::stmi::FloGtkDeviceManager* p0Owner) noexcept
: m_p0Owner(p0Owner)
, m_p0XDisplay(nullptr)
, m_nConnectHandlerDeviceAdded(0)
, m_nConnectHandlerDeviceChanged(0)
, m_nConnectHandlerDeviceRemoved(0)
{
	assert(p0Owner != nullptr);
}
std::string GtkBackend::init(const Glib::RefPtr<Gdk::Display>& refGdkDisplay) noexcept
{
	if (!refGdkDisplay) {
		// Take the default one
		Glib::RefPtr<Gdk::Display> refDefaultDisplay = Gdk::Display::get_default();
		assert(refDefaultDisplay);
		m_refGdkDisplay = refDefaultDisplay;
	} else {
		m_refGdkDisplay = refGdkDisplay;
	}
	//
	GdkDisplay* p0GdkDisplay = m_refGdkDisplay->gobj();
	m_p0XDisplay = ::gdk_x11_display_get_xdisplay(p0GdkDisplay);
	assert(m_p0XDisplay != nullptr);
	// init xi event source
	std::string sError;
	const bool bXIOk = initXI(sError);
	if (!bXIOk) {
		assert(!sError.empty());
		return sError; //-------------------------------------------------------
	}
	initDeviceManager();
	addDevices();
	return "";
}
GtkBackend::~GtkBackend() noexcept
{
	deinitDeviceManager();
}
bool GtkBackend::initXI(std::string& sError) noexcept
{
	int nXIOpcode, nEvent, nError;
	if (!::XQueryExtension( m_p0XDisplay, "XInputExtension", &nXIOpcode, &nEvent, &nError)) {
		sError = "FloGtkDeviceManager: X Input extension not available.";
		return false; //--------------------------------------------------------
	}
	// We need XI 2.2
	int nMajor = 2;
	int nMinor = 2;

	int nRes = ::XIQueryVersion( m_p0XDisplay, &nMajor, &nMinor );
	if ( nRes == BadRequest ) {
		sError = "FloGtkDeviceManager: No XI2 support. Server supports version " + std::to_string(nMajor) + "." + std::to_string(nMinor) + " only.";
		return false; //--------------------------------------------------------
	} else if ( nRes != Success ) {
		sError = "FloGtkDeviceManager: internal XI Error!";
		return false; //--------------------------------------------------------
	} else {
		//std::cout << "FloGtkDeviceManager: XI2 supported. Server provides version " << nMajor << "." << nMinor << '\n';
	}

	::XFlush( m_p0XDisplay );

	m_nXIOpcode = nXIOpcode;
//	assert(!m_refXIEventSource);
//	m_refXIEventSource = Glib::RefPtr<XIEventSource>(new XIEventSource(m_refGdkDisplay.operator->(), nXIOpcode));
//	m_refXIEventSource->connect(sigc::mem_fun(this, &GtkBackend::doXIDeviceEventCallback));
//	m_refXIEventSource->attach();
	return true;
}

void GtkBackend::initDeviceManager() noexcept
{
	assert(m_refGdkDisplay);
	assert(m_nConnectHandlerDeviceAdded == 0);
	auto refGdkDeviceManager = m_refGdkDisplay->get_device_manager();
	::GdkDeviceManager* p0GdkDeviceManager = refGdkDeviceManager->gobj();
	m_nConnectHandlerDeviceAdded = g_signal_connect(p0GdkDeviceManager, "device-added", G_CALLBACK(Private::Flo::gdkDeviceManagerCallbackAdded), this);
	assert(m_nConnectHandlerDeviceAdded > 0);
	m_nConnectHandlerDeviceChanged = g_signal_connect(p0GdkDeviceManager, "device-changed", G_CALLBACK(Private::Flo::gdkDeviceManagerCallbackChanged), this);
	assert(m_nConnectHandlerDeviceChanged > 0);
	m_nConnectHandlerDeviceRemoved = g_signal_connect(p0GdkDeviceManager, "device-removed", G_CALLBACK(Private::Flo::gdkDeviceManagerCallbackRemoved), this);
	assert(m_nConnectHandlerDeviceRemoved > 0);
}
void GtkBackend::deinitDeviceManager() noexcept
{
	if (m_refGdkDisplay && (m_nConnectHandlerDeviceAdded > 0)) {
		auto refGdkDeviceManager = m_refGdkDisplay->get_device_manager();
		::GdkDeviceManager* p0GdkDeviceManager = refGdkDeviceManager->gobj();
		if (::g_signal_handler_is_connected(p0GdkDeviceManager, m_nConnectHandlerDeviceAdded)) {
			::g_signal_handler_disconnect(p0GdkDeviceManager, m_nConnectHandlerDeviceAdded);
		}
		if (::g_signal_handler_is_connected(p0GdkDeviceManager, m_nConnectHandlerDeviceChanged)) {
			::g_signal_handler_disconnect(p0GdkDeviceManager, m_nConnectHandlerDeviceChanged);
		}
		if (::g_signal_handler_is_connected(p0GdkDeviceManager, m_nConnectHandlerDeviceRemoved)) {
			::g_signal_handler_disconnect(p0GdkDeviceManager, m_nConnectHandlerDeviceRemoved);
		}
	}
}
void GtkBackend::addDevices() noexcept
{
	assert(m_refGdkDisplay);
	assert(m_aXDeviceIds.empty());
	assert(m_aGdkDevices.empty());

	auto refGdkDeviceManager = m_refGdkDisplay->get_device_manager();
	const std::vector< Glib::RefPtr< Gdk::Device > > aDevice = refGdkDeviceManager->list_devices(Gdk::DEVICE_TYPE_FLOATING);
	for (auto& refFloatingDevice : aDevice) {
		const Glib::RefPtr<Gdk::Device>& refKeyboard = refFloatingDevice;
		GdkDevice* p0GdkDevice = refKeyboard->gobj();
		addFloatingDevice(p0GdkDevice, false);
	}
}
void GtkBackend::addFloatingDevice(GdkDevice* p0GdkDevice, bool bInformOwner) noexcept
{
	const ::GdkInputSource eSource = ::gdk_device_get_source(p0GdkDevice);
	int32_t nTotKeys = 0;
	if (eSource == ::GDK_SOURCE_KEYBOARD) {
		// add the floating keyboard
		nTotKeys = 1;
	} else {
		// This pointer device might not only have buttons but also keys
		// ex. Logitech Touch Keyboard K400r only appears as Pointer even though it has a keyboard
		nTotKeys = ::gdk_device_get_n_keys(p0GdkDevice);
	}
	if (nTotKeys > 0) {
		const int nXDeviceId = ::gdk_x11_device_get_id(p0GdkDevice);
		m_aXDeviceIds.push_back(nXDeviceId);
		m_aGdkDevices.push_back(p0GdkDevice);
		if (bInformOwner) {
			onDeviceAdded(nXDeviceId);
		}
	}
}
void GtkBackend::removeFloatingDevice(int32_t nIdx) noexcept
{
	onDeviceRemoved(m_aXDeviceIds[nIdx]);
	//
	const int32_t nLastIdx = static_cast<int32_t>(m_aXDeviceIds.size()) - 1;
	if (nIdx < nLastIdx) {
		m_aXDeviceIds[nIdx] = m_aXDeviceIds[nLastIdx];
		m_aGdkDevices[nIdx] = m_aGdkDevices[nLastIdx];
	}
	m_aXDeviceIds.pop_back();
	m_aGdkDevices.pop_back();
}

void GtkBackend::gdkDeviceAdded(
::GdkDeviceManager*
#ifndef NDEBUG
p0GdkDeviceManager
#endif //NDEBUG
, ::GdkDevice* p0Device) noexcept
{
	assert(m_refGdkDisplay);
	#ifndef NDEBUG
	auto refGdkDeviceManager = m_refGdkDisplay->get_device_manager();
	#endif //NDEBUG
	assert(p0GdkDeviceManager == refGdkDeviceManager->gobj());
	::GdkDeviceType eDeviceType = ::gdk_device_get_device_type(p0Device);
	if (eDeviceType != ::GDK_DEVICE_TYPE_FLOATING) {
		// This manager has only floating devices, doesn't care about masters or slaves
		return; //--------------------------------------------------------------
	}
	addFloatingDevice(p0Device, true);
}
void GtkBackend::gdkDeviceChanged(
::GdkDeviceManager*
#ifndef NDEBUG
p0GdkDeviceManager
#endif //NDEBUG
, ::GdkDevice* p0Device) noexcept
{
	assert(m_refGdkDisplay);
	#ifndef NDEBUG
	auto refGdkDeviceManager = m_refGdkDisplay->get_device_manager();
	#endif //NDEBUG
	assert(p0GdkDeviceManager == refGdkDeviceManager->gobj());
	::GdkDeviceType eDeviceType = ::gdk_device_get_device_type(p0Device);
	const int nXDeviceId = ::gdk_x11_device_get_id(p0Device);
	const int32_t nIdx = getXDeviceIdIdx(nXDeviceId);
	const bool bManaged = (nIdx >= 0);
	const bool bIsFloating = (eDeviceType == GDK_DEVICE_TYPE_FLOATING);
	if (bManaged) {
		if (!bIsFloating) {
			removeFloatingDevice(nIdx);
		} else {
			onDeviceChanged(nXDeviceId);
		}
	} else {
		if (bIsFloating) {
			addFloatingDevice(p0Device, true);
		} else {
			// don't care about non floating
			return; //----------------------------------------------------------
		}
	}
}
void GtkBackend::gdkDeviceRemoved(
::GdkDeviceManager *
#ifndef NDEBUG
p0GdkDeviceManager
#endif //NDEBUG
, ::GdkDevice* p0Device) noexcept
{
	assert(m_refGdkDisplay);
	#ifndef NDEBUG
	auto refGdkDeviceManager = m_refGdkDisplay->get_device_manager();
	#endif //NDEBUG
	assert(p0GdkDeviceManager == refGdkDeviceManager->gobj());
	::GdkDeviceType eDeviceType = ::gdk_device_get_device_type(p0Device);
	if (eDeviceType != ::GDK_DEVICE_TYPE_FLOATING) {
		// This manager has only floating devices, doesn't care about masters or slaves
		return; //--------------------------------------------------------------
	}
	const int nXDeviceId = ::gdk_x11_device_get_id(p0Device);
	const int32_t nIdx = getXDeviceIdIdx(nXDeviceId);
	if (nIdx < 0) {
		// It's probably a floating pointer with no keys
		return; //--------------------------------------------------------------
	}
	removeFloatingDevice(nIdx);
}
std::string GtkBackend::getDeviceName(int32_t nXDeviceId) const noexcept
{
	const int32_t nIdx = getXDeviceIdIdx(nXDeviceId);
	if (nIdx < 0) {
		return "";
	}
	return ::gdk_device_get_name(m_aGdkDevices[nIdx]);
}
int32_t GtkBackend::findDevice(::GdkDevice* p0Device, int32_t& nXDeviceId) const noexcept
{
	auto itFind = std::find(m_aGdkDevices.begin(), m_aGdkDevices.end(), p0Device);
	if (itFind == m_aGdkDevices.end()) {
		return -1;
	}
	const int32_t nIdx = std::distance(m_aGdkDevices.begin(), itFind);
	nXDeviceId = m_aXDeviceIds[nIdx];
	return nIdx;
}
int32_t GtkBackend::getXDeviceIdIdx(int32_t nXDeviceId) const noexcept
{
	auto itFind = std::find(m_aXDeviceIds.begin(), m_aXDeviceIds.end(), nXDeviceId);
	if (itFind == m_aXDeviceIds.end()) {
		return -1;
	}
	const int32_t nIdx = std::distance(m_aXDeviceIds.begin(), itFind);
	return nIdx;
}
bool GtkBackend::doXIDeviceEventCallback(::XIDeviceEvent* p0XIDeviceEvent) noexcept
{
//std::cout << "Flo::GtkBackend::doXIDeviceEventCallback(" << reinterpret_cast<int64_t>(this) << ") deviceid=" << p0XIDeviceEvent->deviceid << " nXWinId=" << p0XIDeviceEvent->event << '\n';
	const bool bContinue = true;
	static_assert(sizeof(int32_t) >= sizeof(int), "");
	const int32_t nXDeviceId = p0XIDeviceEvent->deviceid;
	const int32_t nIdx = getXDeviceIdIdx(nXDeviceId);
	if (nIdx < 0) {
//std::cout << "Flo::GtkBackend::doXIDeviceEventCallback   unknown device" << '\n';
		return bContinue; //----------------------------------------------------
	}
	return onXIDeviceEvent(p0XIDeviceEvent);
}

bool GtkBackend::isCompatible(const shared_ptr<GtkAccessor>& refGtkAccessor) const noexcept
{
	Gtk::Window* p0GtkmmWindow = refGtkAccessor->getGtkmmWindow();
	assert(p0GtkmmWindow != nullptr);
	Glib::RefPtr<Gdk::Display> refDisplay = p0GtkmmWindow->get_display();
	assert(refDisplay);
	return (refDisplay == m_refGdkDisplay);
}
void GtkBackend::onDeviceChanged(int32_t nXDeviceId) noexcept
{
	m_p0Owner->onDeviceChanged(nXDeviceId);
}
void GtkBackend::onDeviceAdded(int32_t nXDeviceId) noexcept
{
	m_p0Owner->onDeviceAdded(nXDeviceId);
}
void GtkBackend::onDeviceRemoved(int32_t nXDeviceId) noexcept
{
	m_p0Owner->onDeviceRemoved(nXDeviceId);
}
bool GtkBackend::onXIDeviceEvent(::XIDeviceEvent* p0XIDeviceEvent) noexcept
{
	return m_p0Owner->onXIDeviceEvent(p0XIDeviceEvent);
}
void GtkBackend::initializeGdkWindow(::GdkWindow* p0GdkWindow) noexcept
{
	assert(p0GdkWindow != nullptr);
	::gdk_window_add_filter(p0GdkWindow, gdkWindowEventFilter, this);
}

void GtkBackend::focusDevicesToWindow(const std::shared_ptr<Private::Flo::GtkWindowData>& refWinData) noexcept
{
//std::cout << "Flo::GtkBackend::focusDevicesToWindow  nXWinId=" << nXWinId << '\n';
	Private::Flo::GtkWindowData* p0WinData = refWinData.get();
	const ::Window nXWinId = ((p0WinData != nullptr) ? p0WinData->getXWindow() : None);
	for (int32_t nXDeviceId : m_aXDeviceIds) {
		focusDeviceToXWindow(nXDeviceId, nXWinId);
	}
}
void GtkBackend::focusDeviceToXWindow(int32_t nXDeviceId, ::Window nXWinId) noexcept
{
//std::cout << "Flo::GtkBackend::focusDeviceToXWindow  nXWinId=" << nXWinId << " nDeviceId=" << nXDeviceId << '\n';
	::Window nCheckXWinId;
	Status nCheckStatus = ::XIGetFocus(m_p0XDisplay, nXDeviceId, &nCheckXWinId);
	if ((nCheckStatus == BadValue) || (nCheckStatus == BadWindow) || (nCheckStatus == BadMatch)) {
		std::cout << "FloGtkDeviceManager::focusDeviceToXWindow() XIGetFocus failed for XI device=" << nXDeviceId << '\n';
	}
	if (nCheckXWinId == nXWinId) {
//std::cout << "          already nDeviceId's focus window" << '\n';
		return; //--------------------------------------------------------------
	}
	Status nStatus = ::XISetFocus(m_p0XDisplay, nXDeviceId, nXWinId, CurrentTime);
	if ((nStatus == BadValue) || (nStatus == BadWindow) || (nStatus == BadMatch)) {
		std::cout << "FloGtkDeviceManager::focusDeviceToXWindow() XISetFocus failed for XI device=" << nXDeviceId << "  nXWinId=" << nXWinId << '\n';
	}
}

} // namespace Flo
} // namespace Private

} // namespace stmi
