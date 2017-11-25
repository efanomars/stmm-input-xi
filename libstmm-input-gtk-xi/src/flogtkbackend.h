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
 * File:   flogtkbackend.h
 */

#ifndef STMI_FLO_GTK_BACKEND_H
#define STMI_FLO_GTK_BACKEND_H

#include <stmm-input-gtk/gtkaccessor.h>

#include <gtkmm.h>

#include <cassert>

#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>

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

void gdkDeviceManagerCallbackAdded(GdkDeviceManager *p0DeviceManager, GdkDevice* p0Device, gpointer p0Data);
void gdkDeviceManagerCallbackChanged(GdkDeviceManager *p0DeviceManager, GdkDevice* p0Device, gpointer p0Data);
void gdkDeviceManagerCallbackRemoved(GdkDeviceManager *p0DeviceManager, GdkDevice* p0Device, gpointer p0Data);

////////////////////////////////////////////////////////////////////////////////
class GtkBackend : public sigc::trackable
{
public:
	// throws
	GtkBackend(::stmi::FloGtkDeviceManager* p0Owner, const Glib::RefPtr<Gdk::Display>& refGdkDisplay);
	virtual ~GtkBackend();

#ifdef STMI_TESTING_IFACE
	virtual
#endif
	std::string getDeviceName(int32_t nXDeviceId) const;

#ifdef STMI_TESTING_IFACE
	virtual
#endif
	bool isCompatible(const shared_ptr<GtkAccessor>& refGtkAccessor) const;

#ifdef STMI_TESTING_IFACE
	virtual
#else
	inline
#endif
	const std::vector< int32_t >& getXDeviceIds() { return m_aXDeviceIds; }

#ifdef STMI_TESTING_IFACE
	virtual
#endif
	void focusDevicesToWindow(const std::shared_ptr<Private::Flo::GtkWindowData>& refWinData);

	// This is used by Flo::GtkWindowData
	Display* getXDisplay() { return m_p0XDisplay; }
protected:
	void onDeviceChanged(int32_t nXDeviceId);
	void onDeviceAdded(int32_t nXDeviceId);
	void onDeviceRemoved(int32_t nXDeviceId);
	bool onXIDeviceEvent(XIDeviceEvent* p0XIDeviceEvent);

private:
	friend void Private::Flo::gdkDeviceManagerCallbackAdded(GdkDeviceManager *p0DeviceManager, GdkDevice* p0Device, gpointer p0Data);
	friend void Private::Flo::gdkDeviceManagerCallbackChanged(GdkDeviceManager *p0DeviceManager, GdkDevice* p0Device, gpointer p0Data);
	friend void Private::Flo::gdkDeviceManagerCallbackRemoved(GdkDeviceManager *p0DeviceManager, GdkDevice* p0Device, gpointer p0Data);
	//
	bool initXI(std::string& sError);
	void initDeviceManager();
	void deinitDeviceManager();
	void addDevices();
	void addFloatingDevice(GdkDevice* p0GdkDevice, bool bInformOwner);
	void removeFloatingDevice(int32_t nIdx);
	void gdkDeviceAdded(GdkDeviceManager *p0DeviceManager, GdkDevice* p0Device);
	void gdkDeviceChanged(GdkDeviceManager *p0DeviceManager, GdkDevice* p0Device);
	void gdkDeviceRemoved(GdkDeviceManager *p0DeviceManager, GdkDevice* p0Device);
	int32_t findDevice(GdkDevice* p0Device, int32_t& nXDeviceId) const;
	int32_t getXDeviceIdIdx(int32_t nXDeviceId) const;
	void focusDeviceToXWindow(int32_t nXDeviceId, ::Window nXWinId);

	bool doXIDeviceEventCallback(XIDeviceEvent* p0XIDeviceEvent);
private:
	FloGtkDeviceManager* m_p0Owner;
	Glib::RefPtr<Gdk::Display> m_refGdkDisplay;
	Display* m_p0XDisplay; // shortcut from m_refGdkDisplay
	gulong m_nConnectHandlerDeviceAdded;
	gulong m_nConnectHandlerDeviceChanged;
	gulong m_nConnectHandlerDeviceRemoved;

	Glib::RefPtr<Private::Flo::XIEventSource> m_refXIEventSource;

	// m_aXDeviceIds[nIdx] is the XI device id of gdk device m_aGdkDevices[iIdx]
	std::vector< int32_t > m_aXDeviceIds; // Value: nXdeviceId
	std::vector< GdkDevice* > m_aGdkDevices; // Value: Gdk Device, Size: m_aXDeviceIds.size())
private:
	GtkBackend(const GtkBackend& oSource) = delete;
	GtkBackend& operator=(const GtkBackend& oSource) = delete;
};

} // namespace Flo
} // namespace Private

} // namespace stmi

#endif /* STMI_FLO_GTK_BACKEND_H */
