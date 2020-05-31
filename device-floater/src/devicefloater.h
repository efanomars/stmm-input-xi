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
 * File:   devicefloater.h
 */

#ifndef STMI_DEVICE_FLOATER_H
#define STMI_DEVICE_FLOATER_H

#include <gdkmm.h>

#include <sigc++/signal.h>

#include <string>
#include <vector>

#include <X11/extensions/XInput2.h>
#include <X11/Xlib.h>
#include <stdint.h>

namespace stmi
{

class DeviceFloater
{
public:
	DeviceFloater() noexcept;
	// returns empty string if succesful
	std::string init(const Glib::RefPtr<Gdk::Display>& refDisplay) noexcept;

	~DeviceFloater() noexcept;

	enum DEVICE_TYPE {
		DEVICE_TYPE_FLOATING = 0
		, DEVICE_TYPE_KEYBOARD = 1
		, DEVICE_TYPE_POINTER = 2
	};
	struct SlaveDeviceData
	{
		int32_t m_nId;
		int32_t m_nMasterId;
		std::string m_sName;
		DEVICE_TYPE m_eType;
		int32_t m_nTotKeys;
		bool operator==(const SlaveDeviceData& oOther) const noexcept
		{
			return (m_nId == oOther.m_nId) && (m_sName == oOther.m_sName)
					&& (m_eType == oOther.m_eType) && (m_nTotKeys == oOther.m_nTotKeys);
		}
		bool operator<(const SlaveDeviceData& oOther) const noexcept
		{
			if (m_eType < oOther.m_eType) {
				return true;
			} else if (m_eType > oOther.m_eType) {
				return false;
			}
			if (m_nTotKeys > oOther.m_nTotKeys) { // descending order!
				return true;
			} else if (m_nTotKeys < oOther.m_nTotKeys) {
				return false;
			}
			return (m_nId < oOther.m_nId);
		}
	};
	/* Gets the devices. */
	inline const std::vector<SlaveDeviceData>& getDevices() noexcept { return m_aDevices; }
	/* Floats a device. */
	bool floatDevice(int32_t nDeviceId) noexcept;
	/* Emits when what's returned by getDevices() has changed. */
	sigc::signal<void> m_oDeviceChangedSignal;
	/* Emits when a slave has been added. */
	sigc::signal<void, const SlaveDeviceData&> m_oSlaveAddedSignal;
private:
	std::string initXI(const Glib::RefPtr<Gdk::Display>& refDisplay) noexcept;
	void initGdkDeviceManager(const Glib::RefPtr<Gdk::Display>& refDisplay) noexcept;
	void deinitGdkDeviceManager() noexcept;
	friend void gdkDeviceManagerCallbackCommon(GdkDeviceManager *p0DeviceManager, GdkDevice* p0Device, gpointer p0Data) noexcept;
	void onDeviceCommon(GdkDeviceManager* p0DeviceManager, GdkDevice* p0Device) noexcept;
	//
	/* Reads the devices from XI. */
	void refreshDevices() noexcept;
	void addDevice(const XIDeviceInfo& oDev, DEVICE_TYPE eType) noexcept;
private:
	std::vector<SlaveDeviceData> m_aDevices; // floating first, slaves next, no masters
	Display* m_p0XDisplay;
	Glib::RefPtr<Gdk::DeviceManager> refGdkDeviceManager;
	gulong m_nConnectHandlerDeviceAdded;
	gulong m_nConnectHandlerDeviceChanged;
	gulong m_nConnectHandlerDeviceRemoved;
private:
	DeviceFloater(const DeviceFloater& oSource) = delete;
	DeviceFloater& operator=(const DeviceFloater& oSource) = delete;
};

} // namespace stmi

#endif /* STMI_DEVICE_FLOATER_H */

