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
 * File:   flogtkdevicemanager.h
 */

#ifndef STMI_FLO_GTK_DEVICE_MANAGER_H
#define STMI_FLO_GTK_DEVICE_MANAGER_H

#include <stmm-input-gtk/keyrepeatmode.h>

#include <stmm-input-ev/stddevicemanager.h>

#include <stmm-input/event.h>

#include <gtkmm.h>

#include <vector>
#include <string>
#include <memory>
#include <utility>

#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>

#include <stdint.h>

namespace stmi { class Accessor; }
namespace stmi { class GdkKeyConverter; }

namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

namespace Private
{
namespace Flo
{
	class GtkBackend;
	class GtkWindowData;
	class GtkWindowDataFactory;
	class GtkXKeyboardDevice;
	class XIEventSource;
	class FloGtkListenerExtraData;

	std::string checkIsNotWayland() noexcept;
}
}

/** Handles XInput2 floating devices (both keyboards and pointers) that have keys.
 * An instance only handles the devices of one Gdk::Display.
 *
 * An event (of type stmi::KeyEvent) sent to listeners by this device manager is tied
 * to a Gtk::Window, which has to be added with FloGtkDeviceManager::addAccessor()
 * wrapped in a stmi::GtkAccessor.
 * Events are only sent to the currently active window. When the active window changes
 * cancel events are sent to listeners for each pressed key (tied to the old active window).
 */
class FloGtkDeviceManager : public StdDeviceManager, public sigc::trackable
{
public:
	/** Creates an instance of this class.
 	 * If the passed gdk display is null the system default is used.
	 *
	 * If bEnableEventClasses is `true` then all event classes in aEnDisableEventClasses are enabled, all others disabled,
	 * if `false` then all event classes supported by this instance are enabled except those in aEnDisableEventClasses.
	 * FloGtkDeviceManager doesn't allow disabling event classes once constructed, only enabling.
	 *
	 * Example: To enable all the event classes supported by this instance (currently KeyEvent and DeviceMgmtEvent) pass
	 *
	 *     bEnableEventClasses = false,  aEnDisableEventClasses = {}
	 *
	 * @param bEnableEventClasses Whether to enable or disable all but aEnDisableEventClasses.
	 * @param aEnDisableEventClasses The event classes to be enabled or disabled according to bEnableEventClasses.
	 * @param eKeyRepeatMode Key repeat translation type.
	 * @param refGdkConverter The keycode to hardware key converter. Cannot be null.
	 * @param refDisplay The display used to initialize XInput2. Can be null.
	 * @return The created instance and an empty string or null and an error string if failed (for example because XInput2 is not supported).
	 */
	static std::pair<shared_ptr<FloGtkDeviceManager>, std::string> create(bool bEnableEventClasses, const std::vector<Event::Class>& aEnDisableEventClasses
																		, KeyRepeat::MODE eKeyRepeatMode, const shared_ptr<GdkKeyConverter>& refGdkConverter
																		, const Glib::RefPtr<Gdk::Display>& refDisplay) noexcept;
	/** Creates an instance of this class (as a plug in).
	 *
	 * If bEnableEventClasses is `true` then all event classes in aEnDisableEventClasses are enabled, all others disabled,
	 * if `false` then all event classes supported by this instance are enabled except those in aEnDisableEventClasses.
	 * FloGtkDeviceManager doesn't allow disabling event classes once constructed, only enabling.
	 *
	 * Example: To enable all the event classes supported by this instance (currently just KeyEvent) pass
	 *
	 *     bEnableEventClasses = false,  aEnDisableEventClasses = {}
	 *
	 * This function is used if the device manager is created as a plugin.
	 * The key repeat mode used is the one returned by KeyRepeat::getMode().
	 * The key converter is the one returned by GdkKeyConverter::getConverter().
	 * The display is the one returned by Gdk::Display::get_default().
	 * @param sAppName The application name. Can be empty.
	 * @param bEnableEventClasses Whether to enable or disable all but aEnDisableEventClasses.
	 * @param aEnDisableEventClasses The event classes to be enabled or disabled according to bEnableEventClasses.
	 * @return The created instance and an empty string or null and an error string if failed (for example because XInput2 is not supported).
	 */
	static std::pair<shared_ptr<FloGtkDeviceManager>, std::string> create(const std::string& sAppName
																		, bool bEnableEventClasses, const std::vector<Event::Class>& aEnDisableEventClasses) noexcept;

	void enableEventClass(const Event::Class& oEventClass) noexcept override;

	/** Adds a stmi::GtkAccessor-wrapped Gtk::Window from which events should be received.
	 * An instance of this class needs a stmi::GtkAccessor for each active Gtk::Window
	 * listeners want to receive events from.
	 * If the type of parameter refAccessor is not stmi::GtkAccessor, `false` is returned.
	 * If it is and its Gtk::Window has not the same Gdk::Display as the one passed
	 * as parameter to create the device manager (FloGtkDeviceManager::create()),
	 * `false` is returned.
	 * If the display is the same and the window isn't already added `true` is returned,
	 * `false` otherwise.
	 */
	bool addAccessor(const shared_ptr<Accessor>& refAccessor) noexcept override;
	/** Removes a stmi::GtkAccessor-wrapped Gtk::Window added with addAccessor().
	 * If the device manager has the accessor, `true` is returned, `false` otherwise.
	 *
	 * This function doesn't delete the window itself, it just tells the device manager
	 * to stop tracking it.
	 *
	 * Cancels are sent to listeners for each pressed key.
	 */
	bool removeAccessor(const shared_ptr<Accessor>& refAccessor) noexcept override;
	/** Tells whether a window is already tracked by the device manager.
	 * @param refAccessor The wrapper of the Gtk::Window.
	 * @return Whether the window is currently tracked by the device manager.
	 */
	bool hasAccessor(const shared_ptr<Accessor>& refAccessor) noexcept override;
protected:
	void finalizeListener(ListenerData& oListenerData) noexcept override;
	/** Constructor. */
	FloGtkDeviceManager(bool bEnableEventClasses, const std::vector<Event::Class>& aEnDisableEventClasses
						, KeyRepeat::MODE eKeyRepeatMode, const shared_ptr<GdkKeyConverter>& refGdkConverter) noexcept;
	/** Initializes the device manager. */
	void init(std::unique_ptr<Private::Flo::GtkWindowDataFactory>& refFactory
			, std::unique_ptr<Private::Flo::GtkBackend>& refBackend) noexcept;
private:
	void onDeviceChanged(int32_t nXDeviceId) noexcept;
	void onDeviceAdded(int32_t nXDeviceId) noexcept;
	void onDeviceRemoved(int32_t nXDeviceId) noexcept;
	bool onXIDeviceEvent(XIDeviceEvent* p0XIDeviceEvent) noexcept;

	void adjustConnectionsAfterEnablingClass() noexcept;

	void addDevices() noexcept;
	shared_ptr<Private::Flo::GtkXKeyboardDevice> addFloatingDevice(GdkDevice* p0GdkDevice) noexcept;
	//
	shared_ptr<Private::Flo::GtkXKeyboardDevice> addGtkXKeyboardDevice(int nXDeviceId, const Glib::ustring& sDeviceName) noexcept;
	void removeGtkXKeyboardDevice(int32_t nIdx) noexcept;

	bool findDevice(int32_t nXDeviceId
				, std::vector< std::pair<int32_t, shared_ptr<Private::Flo::GtkXKeyboardDevice> > >::iterator& itFind) noexcept;
	bool findWindow(Gtk::Window* p0GtkmmWindow
				, std::vector< std::pair<Gtk::Window*, shared_ptr<Private::Flo::GtkWindowData> > >::iterator& itFind) noexcept;
	bool hasAccessor(const shared_ptr<Accessor>& refAccessor, bool& bValid
					, std::vector<std::pair<Gtk::Window*, shared_ptr<Private::Flo::GtkWindowData> > >::iterator& itFind) noexcept;
	void removeAccessor(const std::vector<std::pair<Gtk::Window*, shared_ptr<Private::Flo::GtkWindowData> > >::iterator& itGtkData) noexcept;
	void onIsActiveChanged(const shared_ptr<Private::Flo::GtkWindowData>& refWindowData) noexcept;
	std::shared_ptr<Private::Flo::GtkWindowData> getGtkWindowData() noexcept;

	void connectDeviceToAllWindows(int32_t nXDeviceId) noexcept;

	void selectAccessor(const shared_ptr<Private::Flo::GtkWindowData>& refData) noexcept;
	void cancelDeviceKeys(const shared_ptr<Private::Flo::GtkXKeyboardDevice>& refGtkXKeyboard) noexcept;
	void deselectAccessor() noexcept;
	void focusSelectedWindow() noexcept;

	friend class Private::Flo::GtkBackend;
	friend class Private::Flo::GtkWindowData;
	friend class Private::Flo::GtkXKeyboardDevice;
	friend class Private::Flo::XIEventSource;
	friend class Private::Flo::FloGtkListenerExtraData;
private:
	std::unique_ptr<Private::Flo::GtkWindowDataFactory> m_refFactory;
	std::unique_ptr<Private::Flo::GtkBackend> m_refBackend;

	// The GtkAccessor (GtkWindowData::m_refAccessor) will tell
	// when the window gets deleted. The accessor can also be removed
	// explicitely during a listener callback.
	std::vector<std::pair<Gtk::Window*, shared_ptr<Private::Flo::GtkWindowData> > > m_aGtkWindowData;
	// The currently active accessor (window), can be null.
	std::shared_ptr<Private::Flo::GtkWindowData> m_refSelected;
	// Invariants:
	// - if m_refSelected is null, no key is pressed in any of the devices
	// - if m_refSelected is not null, all pressed keys in any of the devices
	//   were generated for the selected window.

	int32_t m_nCancelingNestedDepth;

	std::vector< std::pair<int32_t, shared_ptr<Private::Flo::GtkXKeyboardDevice> > > m_aKeyboardDevices; // Value: (X device id, device)

	KeyRepeat::MODE m_eKeyRepeatMode;

	const shared_ptr<GdkKeyConverter> m_refGdkConverter;
	// Fast access reference to converter
	const GdkKeyConverter& m_oConverter;
	//
	const int32_t m_nClassIdxKeyEvent;
private:
	FloGtkDeviceManager() = delete;
	FloGtkDeviceManager(const FloGtkDeviceManager& oSource) = delete;
	FloGtkDeviceManager& operator=(const FloGtkDeviceManager& oSource) = delete;
};

} // namespace stmi

#endif /* STMI_FLO_GTK_DEVICE_MANAGER_H */
