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
 * File:   flogtkwindowdata.h
 */

#ifndef STMI_FLO_GTK_WINDOW_DATA_H
#define STMI_FLO_GTK_WINDOW_DATA_H

#include "flogtkdevicemanager.h"

#include "recycler.h"

#include <gtkmm.h>

#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>


namespace stmi
{

namespace Private
{
namespace Flo
{

using std::shared_ptr;
using std::weak_ptr;

////////////////////////////////////////////////////////////////////////////////
class GtkWindowData : public std::enable_shared_from_this<GtkWindowData>, public sigc::trackable
{
public:
	GtkWindowData()
	: m_p0Owner(nullptr)
	, m_p0XDisplay(nullptr)
	, m_nXWinId(None)
	, m_bIsEnabled(false)
	, m_bIsRealized(false)
	{
	}
	virtual ~GtkWindowData()
	{
		disable();
	}
	void reInit()
	{
	}
	//
	#ifdef STMI_TESTING_IFACE
	virtual
	#endif
	void enable(const shared_ptr<GtkAccessor>& refAccessor, FloGtkDeviceManager* p0Owner);
	#ifdef STMI_TESTING_IFACE
	virtual
	#endif
	void disable();
	#ifdef STMI_TESTING_IFACE
	virtual
	#else
	inline
	#endif
	bool isEnabled() const { return m_bIsEnabled; }

	#ifdef STMI_TESTING_IFACE
	virtual
	#else
	inline
	#endif
	void connectAllDevices()
	{
		checkXWin();
		if (m_nXWinId != None) {
			setXIEventsForAllDevices(true);
		}
	}
	#ifdef STMI_TESTING_IFACE
	virtual
	#else
	inline
	#endif
	void connectDevice(int32_t nXDeviceId)
	{
		checkXWin();
		if (m_nXWinId != None) {
			setXIEventsForDevice(nXDeviceId, true);
		}
	}

	#ifdef STMI_TESTING_IFACE
	virtual
	#else
	inline
	#endif
	const shared_ptr<GtkAccessor>& getAccessor() { return m_refAccessor; }

	#ifdef STMI_TESTING_IFACE
	virtual
	#else
	inline
	#endif
	bool isWindowActive() const
	{
		assert(m_refAccessor);
		auto p0GtkmmWindow = m_refAccessor->getGtkmmWindow();
		return p0GtkmmWindow->get_realized() && p0GtkmmWindow->get_visible() && p0GtkmmWindow->is_active();
	}
	//
	#ifdef STMI_TESTING_IFACE
	virtual
	#else
	inline 
	#endif
	::Window getXWindow() const { return m_nXWinId; }
	//
protected:
	inline void setOwner(FloGtkDeviceManager* p0Owner)
	{
		m_p0Owner = p0Owner;
	}
	//
	void onSigIsActiveChanged() { m_p0Owner->onIsActiveChanged(shared_from_this()); }
private:
	void checkXWin()
	{
		if (m_nXWinId != None) {
			return;
		}
		if (m_refAccessor->isDeleted())  {
			return;
		}
		// When added the window wasn't realized
		setXWinAndXDisplay(m_refAccessor->getGtkmmWindow());
	}
	void setXWinAndXDisplay(Gtk::Window* p0GtkmmWindow);

	inline void disconnectAllDevices()
	{
		checkXWin();
		setXIEventsForAllDevices(false);
	}
	void setXIEventsForAllDevices(bool bSet);
	// returns true if connected all because just realized
	bool setXIEventsForDevice(int32_t nXDeviceId, bool bSet);
private:
	FloGtkDeviceManager* m_p0Owner;
	// Although the display is also stored in the backend, it's better to
	// keep a ref here so that while disconnecting in destructor we don't
	// have to rely on the backend to still be there.
	Glib::RefPtr<Gdk::Display> m_refGdkDisplay;
	Display* m_p0XDisplay; // shortcut from m_refGdkDisplay
	shared_ptr<GtkAccessor> m_refAccessor;
	::Window m_nXWinId;
	//
	bool m_bIsEnabled;
	bool m_bIsRealized;

	sigc::connection m_oIsActiveConn;
};

////////////////////////////////////////////////////////////////////////////////
class GtkWindowDataFactory
{
public:
	#ifdef STMI_TESTING_IFACE
	virtual
	#endif
	shared_ptr<GtkWindowData> create()
	{
		shared_ptr<GtkWindowData> ref;
		m_oRecycler.create(ref);
		return ref;
	}
	#ifdef STMI_TESTING_IFACE
	virtual ~GtkWindowDataFactory() = default;
	#endif
private:
	Recycler<GtkWindowData> m_oRecycler;
};

} // namespace Flo
} // namespace Private

} // namespace stmi

#endif /* STMI_FLO_GTK_WINDOW_DATA_H */
