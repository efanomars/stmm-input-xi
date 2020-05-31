/*
 * Copyright © 2016-2020  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   devicefloater.cc
 */
#include "devicefloater.h"

#include <gdk/gdkx.h>

#include <X11/extensions/XI2.h>

#include <cassert>
#ifndef NDEBUG
//#include <iostream>
#endif //NDEBUG
#include <algorithm>

#include <X11/X.h>

namespace stmi
{

DeviceFloater::DeviceFloater() noexcept
: m_p0XDisplay(nullptr)
, m_nConnectHandlerDeviceAdded(0)
, m_nConnectHandlerDeviceChanged(0)
, m_nConnectHandlerDeviceRemoved(0)
{
}
std::string DeviceFloater::init(const Glib::RefPtr<Gdk::Display>& refDisplay) noexcept
{
	assert(refDisplay);
	const std::string sError = initXI(refDisplay);
	if (sError.empty()) {
		// no errors
		initGdkDeviceManager(refDisplay);
		refreshDevices();
	}
	return sError;
}
DeviceFloater::~DeviceFloater() noexcept
{
	deinitGdkDeviceManager();
}
std::string DeviceFloater::initXI(const Glib::RefPtr<Gdk::Display>& refDisplay) noexcept
{
	GdkDisplay* p0GdkDisplay = refDisplay->gobj();
	Display* p0XDisplay = ::gdk_x11_display_get_xdisplay(p0GdkDisplay);
	int nXIOpcode;
	int nEvent;
	int nError;
	if (!::XQueryExtension( p0XDisplay, "XInputExtension", &nXIOpcode, &nEvent, &nError)) {
		return "DeviceFloater: X Input extension not available.";
	}
	// We need XI 2.2
	int nMajor = 2;
	int nMinor = 2;

	int nRes = ::XIQueryVersion( p0XDisplay, &nMajor, &nMinor );
	if ( nRes == BadRequest ) {
		return "DeviceFloater: No XI2 support. Server supports version " + std::to_string(nMajor) + "." + std::to_string(nMinor) + " only.";
	} else if ( nRes != Success ) {
		return "DeviceFloater: Internal Error! Probably a bug in Xlib.";
	//} else {
	//	std::cout << "DeviceFloater: XI2 supported. Server provides version " << nMajor << "." << nMinor << std::endl;
	}
	m_p0XDisplay = p0XDisplay;
	return "";
}
void gdkDeviceManagerCallbackCommon(GdkDeviceManager *p0DeviceManager, GdkDevice* p0Device, gpointer p0Data) noexcept
{
	auto p0DeviceFloater = static_cast<DeviceFloater*>(p0Data);
	p0DeviceFloater->onDeviceCommon(p0DeviceManager, p0Device);
}
void DeviceFloater::initGdkDeviceManager(const Glib::RefPtr<Gdk::Display>& refDisplay) noexcept
{
	refGdkDeviceManager = refDisplay->get_device_manager();
	assert(m_nConnectHandlerDeviceAdded == 0);
	GdkDeviceManager* p0GdkDeviceManager = refGdkDeviceManager->gobj();
	gpointer p0Data = this;
	m_nConnectHandlerDeviceAdded = g_signal_connect(p0GdkDeviceManager, "device-added", G_CALLBACK(gdkDeviceManagerCallbackCommon), p0Data);
	assert(m_nConnectHandlerDeviceAdded > 0);
	m_nConnectHandlerDeviceChanged = g_signal_connect(p0GdkDeviceManager, "device-changed", G_CALLBACK(gdkDeviceManagerCallbackCommon), p0Data);
	assert(m_nConnectHandlerDeviceChanged > 0);
	m_nConnectHandlerDeviceRemoved = g_signal_connect(p0GdkDeviceManager, "device-removed", G_CALLBACK(gdkDeviceManagerCallbackCommon), p0Data);
	assert(m_nConnectHandlerDeviceRemoved > 0);
}
void DeviceFloater::deinitGdkDeviceManager() noexcept
{
	if (refGdkDeviceManager && (m_nConnectHandlerDeviceAdded > 0)) {
		GdkDeviceManager* p0GdkDeviceManager = refGdkDeviceManager->gobj();
		if (g_signal_handler_is_connected(p0GdkDeviceManager, m_nConnectHandlerDeviceAdded)) {
			g_signal_handler_disconnect(p0GdkDeviceManager, m_nConnectHandlerDeviceAdded);
		}
		if (g_signal_handler_is_connected(p0GdkDeviceManager, m_nConnectHandlerDeviceChanged)) {
			g_signal_handler_disconnect(p0GdkDeviceManager, m_nConnectHandlerDeviceChanged);
		}
		if (g_signal_handler_is_connected(p0GdkDeviceManager, m_nConnectHandlerDeviceRemoved)) {
			g_signal_handler_disconnect(p0GdkDeviceManager, m_nConnectHandlerDeviceRemoved);
		}
	}
}
void DeviceFloater::onDeviceCommon(GdkDeviceManager*
									#ifndef NDEBUG
									p0DeviceManager
									#endif //NDEBUG
									, GdkDevice* p0Device) noexcept
{
	assert(p0DeviceManager == refGdkDeviceManager->gobj());
	GdkDeviceType eDeviceType = ::gdk_device_get_device_type(p0Device);
	if (eDeviceType == GDK_DEVICE_TYPE_MASTER) {
		// no masters, just slaves (floating or not)
		return; //--------------------------------------------------------------
	}
	auto aSaveDevices = m_aDevices;
	refreshDevices();
	if (aSaveDevices == m_aDevices) {
		return; //--------------------------------------------------------------
	}
	m_oDeviceChangedSignal.emit();
	// for each slave not in saved devices call
	decltype(aSaveDevices) aAddedSlaves;
	for (auto& oData : m_aDevices) {
		auto itSaved = std::find_if(aSaveDevices.begin(), aSaveDevices.end(),
				[&oData](const SlaveDeviceData& oOldData)
				{
					return oData.m_nId == oOldData.m_nId;
				}
				);
		if (itSaved == aSaveDevices.end()) {
			aAddedSlaves.push_back(oData);
		}
	}
	for (auto& oData : aAddedSlaves) {
		m_oSlaveAddedSignal.emit(oData);
	}
}
void DeviceFloater::addDevice(const XIDeviceInfo& oDev, DEVICE_TYPE eType) noexcept
{
	SlaveDeviceData oData;
	oData.m_nId = oDev.deviceid;
	oData.m_nMasterId = ((eType == DEVICE_TYPE_FLOATING) ? -777 : oDev.attachment);
	oData.m_eType = eType;
	oData.m_sName = oDev.name;
	oData.m_nTotKeys = 0;
	XIAnyClassInfo** p0Classes = oDev.classes;
	const int32_t nTotClasses = oDev.num_classes;
	for (int32_t nCurClass = 0; nCurClass < nTotClasses; ++nCurClass) {
		if (p0Classes[nCurClass]->type == XIKeyClass) {
			XIKeyClassInfo *p0KeyClass = reinterpret_cast<XIKeyClassInfo*>(p0Classes[nCurClass]);
			oData.m_nTotKeys = std::max(p0KeyClass->num_keycodes, oData.m_nTotKeys);
		}
	}
	m_aDevices.emplace_back(oData);
}
void DeviceFloater::refreshDevices() noexcept
{
	static_assert(sizeof(int) == sizeof(int32_t), "");

	m_aDevices.clear();

	assert(m_p0XDisplay != nullptr);
	int32_t nTotDevices;
	XIDeviceInfo* pInfo = ::XIQueryDevice(m_p0XDisplay, XIAllDevices, &nTotDevices);
//std::cout << "DeviceFloater::refreshDevices()  nTotDevices=" << nTotDevices << std::endl;

	// Add floating devices
	for (int32_t nCurDevice = 0; nCurDevice < nTotDevices; ++nCurDevice) {
		const XIDeviceInfo& oDev = pInfo[nCurDevice];
		if (oDev.enabled == 0) {
			continue; //for
		}
		DEVICE_TYPE eType = DEVICE_TYPE_FLOATING;
		if (oDev.use == XISlaveKeyboard) {
			eType = DEVICE_TYPE_KEYBOARD;
		} else if (oDev.use == XISlavePointer) {
			eType = DEVICE_TYPE_POINTER;
		} else if (oDev.use == XIFloatingSlave) {
		} else {
			continue; //for
		}
		addDevice(oDev, eType);
	}
	::XIFreeDeviceInfo(pInfo);
	//
	std::sort(m_aDevices.begin(), m_aDevices.end());
}
bool DeviceFloater::floatDevice(int32_t nDeviceId) noexcept
{
	XIDetachSlaveInfo oDetach;

	oDetach.type = XIDetachSlave;
	oDetach.deviceid = nDeviceId;

	const int32_t nRet = ::XIChangeHierarchy(m_p0XDisplay, reinterpret_cast<XIAnyHierarchyChangeInfo*>(&oDetach), 1);
	return (nRet == 0);
}

} // namespace stmi

