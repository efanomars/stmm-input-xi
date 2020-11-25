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
 * File:   flogtkbackend.h
 */

#ifndef STMI_FLO_GTK_BACKEND_H
#define STMI_FLO_GTK_BACKEND_H

#include <gtkmm.h>

//#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include <X11/extensions/XInput2.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include <stdint.h>

namespace stmi { class GtkAccessor; }

namespace stmi
{

class FloGtkDeviceManager;

namespace Private
{
namespace Flo
{

class XIEventSource;
class GtkWindowData;

using std::shared_ptr;
using std::weak_ptr;

void gdkDeviceManagerCallbackAdded(::GdkDeviceManager* p0DeviceManager, ::GdkDevice* p0Device, ::gpointer p0Data) noexcept;
void gdkDeviceManagerCallbackChanged(::GdkDeviceManager* p0DeviceManager, ::GdkDevice* p0Device, ::gpointer p0Data) noexcept;
void gdkDeviceManagerCallbackRemoved(::GdkDeviceManager* p0DeviceManager, ::GdkDevice* p0Device, ::gpointer p0Data) noexcept;
::GdkFilterReturn gdkWindowEventFilter(::GdkXEvent* p0XEevent, ::GdkEvent* /*p0Event*/, ::gpointer p0Data) noexcept;

////////////////////////////////////////////////////////////////////////////////
class GtkBackend : public sigc::trackable
{
public:
	explicit GtkBackend(::stmi::FloGtkDeviceManager* p0Owner) noexcept;
	// return empty string if successful, error otherwise
	std::string init(const Glib::RefPtr<Gdk::Display>& refGdkDisplay) noexcept;

	#ifdef STMI_TESTING_IFACE
	virtual
	#endif
	~GtkBackend() noexcept;

	#ifdef STMI_TESTING_IFACE
	virtual
	#endif
	std::string getDeviceName(int32_t nXDeviceId) const noexcept;

	#ifdef STMI_TESTING_IFACE
	virtual
	#endif
	bool isCompatible(const shared_ptr<GtkAccessor>& refGtkAccessor) const noexcept;

	#ifdef STMI_TESTING_IFACE
	virtual
	#else
	inline
	#endif
	const std::vector< int32_t >& getXDeviceIds() noexcept { return m_aXDeviceIds; }

	#ifdef STMI_TESTING_IFACE
	virtual
	#endif
	void focusDevicesToWindow(const std::shared_ptr<Private::Flo::GtkWindowData>& refWinData) noexcept;

	// This is used by Flo::GtkWindowData
	::Display* getXDisplay() noexcept { return m_p0XDisplay; }

	void initializeGdkWindow(::GdkWindow* p0GdkWindow) noexcept;
protected:
	void onDeviceChanged(int32_t nXDeviceId) noexcept;
	void onDeviceAdded(int32_t nXDeviceId) noexcept;
	void onDeviceRemoved(int32_t nXDeviceId) noexcept;
	bool onXIDeviceEvent(::XIDeviceEvent* p0XIDeviceEvent) noexcept;

private:
	friend void Private::Flo::gdkDeviceManagerCallbackAdded(::GdkDeviceManager* p0DeviceManager, ::GdkDevice* p0Device, ::gpointer p0Data) noexcept;
	friend void Private::Flo::gdkDeviceManagerCallbackChanged(::GdkDeviceManager* p0DeviceManager, ::GdkDevice* p0Device, ::gpointer p0Data) noexcept;
	friend void Private::Flo::gdkDeviceManagerCallbackRemoved(::GdkDeviceManager* p0DeviceManager, ::GdkDevice* p0Device, ::gpointer p0Data) noexcept;
	friend GdkFilterReturn Private::Flo::gdkWindowEventFilter(::GdkXEvent* p0XEevent, ::GdkEvent* p0Event, ::gpointer p0Data) noexcept;
	//
	bool initXI(std::string& sError) noexcept;
	void initDeviceManager() noexcept;
	void deinitDeviceManager() noexcept;
	void addDevices() noexcept;
	void addFloatingDevice(::GdkDevice* p0GdkDevice, bool bInformOwner) noexcept;
	void removeFloatingDevice(int32_t nIdx) noexcept;
	void gdkDeviceAdded(::GdkDeviceManager* p0DeviceManager, ::GdkDevice* p0Device) noexcept;
	void gdkDeviceChanged(::GdkDeviceManager* p0DeviceManager, ::GdkDevice* p0Device) noexcept;
	void gdkDeviceRemoved(::GdkDeviceManager* p0DeviceManager, ::GdkDevice* p0Device) noexcept;
	int32_t findDevice(::GdkDevice* p0Device, int32_t& nXDeviceId) const noexcept;
	int32_t getXDeviceIdIdx(int32_t nXDeviceId) const noexcept;
	void focusDeviceToXWindow(int32_t nXDeviceId, ::Window nXWinId) noexcept;

	bool doXIDeviceEventCallback(::XIDeviceEvent* p0XIDeviceEvent) noexcept;
private:
	FloGtkDeviceManager* m_p0Owner;
	Glib::RefPtr<Gdk::Display> m_refGdkDisplay;
	::Display* m_p0XDisplay; // shortcut from m_refGdkDisplay
	gulong m_nConnectHandlerDeviceAdded;
	gulong m_nConnectHandlerDeviceChanged;
	gulong m_nConnectHandlerDeviceRemoved;

	int32_t m_nXIOpcode;

	// m_aXDeviceIds[nIdx] is the XI device id of gdk device m_aGdkDevices[nIdx]
	std::vector< int32_t > m_aXDeviceIds; // Value: nXdeviceId
	std::vector< ::GdkDevice* > m_aGdkDevices; // Value: Gdk Device, Size: m_aXDeviceIds.size())
private:
	GtkBackend(const GtkBackend& oSource) = delete;
	GtkBackend& operator=(const GtkBackend& oSource) = delete;
};

} // namespace Flo
} // namespace Private

} // namespace stmi

#endif /* STMI_FLO_GTK_BACKEND_H */
