/*
 * Copyright © 2016-2017  Stefano Marsili, <stemars@gmx.ch>
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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   devicefloater.h
 */

#ifndef STMI_DEVICE_FLOATER_H
#define STMI_DEVICE_FLOATER_H

#include <gtkmm.h>

#include <sigc++/signal.h>

#include <X11/extensions/XInput2.h>

namespace stmi
{

class DeviceFloater
{
public:
	/* throws */
	DeviceFloater(const Glib::RefPtr<Gdk::Display>& refDisplay);
	virtual ~DeviceFloater();

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
		bool operator==(const SlaveDeviceData& oOther) const
		{
			return (m_nId == oOther.m_nId) && (m_sName == oOther.m_sName)
					&& (m_eType == oOther.m_eType) && (m_nTotKeys == oOther.m_nTotKeys);
		}
		bool operator<(const SlaveDeviceData& oOther) const
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
	inline const std::vector<SlaveDeviceData>& getDevices() { return m_aDevices; }
	/* Floats a device. */
	bool floatDevice(int32_t nDeviceId);
	/* Emits when what's returned by getDevices() has changed. */
	sigc::signal<void> m_oDeviceChangedSignal;
	/* Emits when a slave has been added. */
	sigc::signal<void, const SlaveDeviceData&> m_oSlaveAddedSignal;
private:
	void initXI(const Glib::RefPtr<Gdk::Display>& refDisplay);
	void initGdkDeviceManager(const Glib::RefPtr<Gdk::Display>& refDisplay);
	void deinitGdkDeviceManager();
	friend void gdkDeviceManagerCallbackCommon(GdkDeviceManager *p0DeviceManager, GdkDevice* p0Device, gpointer p0Data);
	void onDeviceCommon(GdkDeviceManager* p0DeviceManager, GdkDevice* p0Device);
	//
	/* Reads the devices from XI. */
	void refreshDevices();
	void addDevice(const XIDeviceInfo& oDev, DEVICE_TYPE eType);
private:
	std::vector<SlaveDeviceData> m_aDevices; // floating first, slaves next, no masters
	Display* m_p0XDisplay;
	Glib::RefPtr<Gdk::DeviceManager> refGdkDeviceManager;
	gulong m_nConnectHandlerDeviceAdded;
	gulong m_nConnectHandlerDeviceChanged;
	gulong m_nConnectHandlerDeviceRemoved;
};

} // namespace stmi

#endif /* STMI_DEVICE_FLOATER_H */

