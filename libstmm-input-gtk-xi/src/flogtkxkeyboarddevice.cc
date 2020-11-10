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
 * File:   flogtkxkeyboarddevice.cc
 */

#include "flogtkxkeyboarddevice.h"

#include "flogtkwindowdata.h"
#include "flogtklistenerextradata.h"

#include <stmm-input-base/basicdevicemanager.h>
#include <stmm-input-gtk/gdkkeyconverter.h>
#include <stmm-input-gtk/keyrepeatmode.h>
#include <stmm-input/devicemanager.h>
#include <stmm-input/event.h>

#include <cstdint>
#include <limits>
#include <utility>
#include <cassert>

#include <X11/extensions/XI2.h>

namespace stmi { class Device; }
namespace stmi { class GtkAccessor; }

namespace stmi
{

namespace Private
{
namespace Flo
{

shared_ptr<Device> GtkXKeyboardDevice::getDevice() const noexcept
{
	shared_ptr<const GtkXKeyboardDevice> refConstThis = shared_from_this();
	shared_ptr<GtkXKeyboardDevice> refThis = std::const_pointer_cast<GtkXKeyboardDevice>(refConstThis);
	return refThis;
}
shared_ptr<Capability> GtkXKeyboardDevice::getCapability(const Capability::Class& oClass) const noexcept
{
	shared_ptr<Capability> refCapa;
	if (oClass == typeid(KeyCapability)) {
		shared_ptr<const GtkXKeyboardDevice> refConstThis = shared_from_this();
		shared_ptr<GtkXKeyboardDevice> refThis = std::const_pointer_cast<GtkXKeyboardDevice>(refConstThis);
		refCapa = refThis;
	}
	return refCapa;
}
shared_ptr<Capability> GtkXKeyboardDevice::getCapability(int32_t nCapabilityId) const noexcept
{
	const auto nKeyCapaId = KeyCapability::getId();
	if (nCapabilityId != nKeyCapaId) {
		return shared_ptr<Capability>{};
	}
	shared_ptr<const GtkXKeyboardDevice> refConstThis = shared_from_this();
	shared_ptr<GtkXKeyboardDevice> refThis = std::const_pointer_cast<GtkXKeyboardDevice>(refConstThis);
	return refThis;
}
std::vector<int32_t> GtkXKeyboardDevice::getCapabilities() const noexcept
{
	return {KeyCapability::getId()};
}
std::vector<Capability::Class> GtkXKeyboardDevice::getCapabilityClasses() const noexcept
{
	return {KeyCapability::getClass()};
}
bool GtkXKeyboardDevice::isKeyboard() const noexcept
{
	return true;
}
bool GtkXKeyboardDevice::handleXIDeviceEvent(::XIDeviceEvent* p0XIDeviceEvent, const shared_ptr<GtkWindowData>& refWindowData) noexcept
{
//std::cout << "GtkXKeyboardDevice::handleXIDeviceEvent()" << '\n';
	const bool bContinue = true;
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return !bContinue; //---------------------------------------------------
	}
	//
	FloGtkDeviceManager* p0Owner = refOwner.get();
	#ifndef NDEBUG
	if (p0Owner->m_refSelected != refWindowData) {
		//TODO this shouldn't happen
//std::cout << "GtkXKeyboardDevice::handleXIDeviceEvent() wrong window" << '\n';
		return bContinue; //----------------------------------------------------
	}
	#endif //NDEBUG
	HARDWARE_KEY eHardwareKey;
	if (!p0Owner->m_oConverter.convertKeyCodeToHardwareKey(static_cast<guint16>(p0XIDeviceEvent->detail), eHardwareKey)) {
//std::cout << "GtkXKeyboardDevice::handleXIDeviceEvent() could not convert key" << '\n';
		return bContinue; //----------------------------------------------------
	}
	auto refListeners = p0Owner->getListeners();
	const int32_t nEvType = p0XIDeviceEvent->evtype;
	uint64_t nPressedTimeStamp = std::numeric_limits<uint64_t>::max();
	auto itFind = m_oPressedKeys.find(eHardwareKey);
	const bool bHardwareKeyPressed = (itFind != m_oPressedKeys.end());
	KeyEvent::KEY_INPUT_TYPE eInputType;
	shared_ptr<GtkXKeyboardDevice> refGtkXKeyboardDevice = shared_from_this();
	shared_ptr<KeyCapability> refCapability = refGtkXKeyboardDevice;
	auto refWindowAccessor = refWindowData->getAccessor();
	assert(refWindowAccessor);
	if (nEvType == XI_KeyPress) {
		if (bHardwareKeyPressed) {
			// Key repeat
			KeyData& oKeyData = itFind->second;
			if (refOwner->m_eKeyRepeatMode == KeyRepeat::MODE_SUPPRESS) {
				return bContinue; //--------------------------------------------
			}
			KeyEvent::KEY_INPUT_TYPE eAddInputType;
			if (refOwner->m_eKeyRepeatMode == KeyRepeat::MODE_ADD_RELEASE) {
				eAddInputType = KeyEvent::KEY_RELEASE;
			} else {
				eAddInputType = KeyEvent::KEY_RELEASE_CANCEL;
			}
			const auto nTimePressedStamp = oKeyData.m_nPressedTimeStamp;
			m_oPressedKeys.erase(itFind);
			shared_ptr<Event> refEvent;
			const int64_t nEventTimeUsec = DeviceManager::getNowTimeMicroseconds();
			for (auto& p0ListenerData : *refListeners) {
				sendKeyEventToListener(*p0ListenerData, nEventTimeUsec, nTimePressedStamp, eAddInputType, eHardwareKey
										, refWindowAccessor, refCapability, p0Owner->m_nClassIdxKeyEvent, refEvent);
			}
			if (!refWindowData->isEnabled()) {
				// the accessor was removed during the callbacks!
//std::cout << "GtkXKeyboardDevice::handleXIDeviceEvent accessor  removed???" << '\n';
				return bContinue; //--------------------------------------------
			}
		}
		nPressedTimeStamp = FloGtkDeviceManager::getUniqueTimeStamp();
		KeyData oKeyData;
		oKeyData.m_nPressedTimeStamp = nPressedTimeStamp;
		m_oPressedKeys.emplace(eHardwareKey, oKeyData);
		eInputType = KeyEvent::KEY_PRESS;
	} else {
		assert(nEvType == XI_KeyRelease);
		if (!bHardwareKeyPressed) {
			// orphan release, ignore
//std::cout << "GtkXKeyboardDevice::handleXIDeviceEvent orphan release" << '\n';
			return bContinue; //------------------------------------------------
		}
		KeyData& oKeyData = itFind->second;
		nPressedTimeStamp = oKeyData.m_nPressedTimeStamp;
		eInputType = KeyEvent::KEY_RELEASE;
		m_oPressedKeys.erase(itFind);
	}
	shared_ptr<Event> refEvent;
	const int64_t nEventTimeUsec = DeviceManager::getNowTimeMicroseconds();
	for (auto& p0ListenerData : *refListeners) {
		sendKeyEventToListener(*p0ListenerData, nEventTimeUsec, nPressedTimeStamp, eInputType, eHardwareKey
								, refWindowAccessor, refCapability, p0Owner->m_nClassIdxKeyEvent, refEvent);
		if ((eInputType == KeyEvent::KEY_PRESS) && (m_oPressedKeys.find(eHardwareKey) == m_oPressedKeys.end())) {
			// The key was canceled by the callback
			break; // for -------
		}
	}
	return bContinue;
}
void GtkXKeyboardDevice::finalizeListener(FloGtkDeviceManager::ListenerData& oListenerData, int64_t nEventTimeUsec) noexcept
{
//std::cout << "GtkXKeyboardDevice::finalizeListener m_oPressedKeys.size()=" << m_oPressedKeys.size() << '\n';
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return;
	}
	FloGtkDeviceManager* p0Owner = refOwner.get();
	if (!p0Owner->isEventClassEnabled(Event::Class{typeid(KeyEvent)})) {
		return; //--------------------------------------------------------------
	}
	auto& refSelected = p0Owner->m_refSelected;
	if (!refSelected) {
		return; //--------------------------------------------------------------
	}
	auto refSelectedAccessor = refSelected->getAccessor();

	shared_ptr<GtkXKeyboardDevice> refGtkXKeyboardDevice = shared_from_this();
	shared_ptr<KeyCapability> refCapability = refGtkXKeyboardDevice;

	FloGtkListenerExtraData* p0ExtraData = nullptr;
	oListenerData.getExtraData(p0ExtraData);

	// Send KEY_RELEASE_CANCEL for the currently pressed keys to the listener
	// work on copy
	auto oPressedKeys = m_oPressedKeys;
	for (auto& oPair : oPressedKeys) {
		const int32_t nHardwareKey = oPair.first;
		const KeyData& oKeyData = oPair.second;
		//
		const auto nKeyPressedTimeStamp = oKeyData.m_nPressedTimeStamp;
		const HARDWARE_KEY eHardwareKey = static_cast<HARDWARE_KEY>(nHardwareKey);
		//
		if (p0ExtraData->isKeyCanceled(nHardwareKey)) {
			continue; // for ------------
		}
		p0ExtraData->setKeyCanceled(nHardwareKey);
		//
		shared_ptr<Event> refEvent;
		sendKeyEventToListener(oListenerData, nEventTimeUsec, nKeyPressedTimeStamp, KeyEvent::KEY_RELEASE_CANCEL
								, eHardwareKey, refSelectedAccessor, refCapability, p0Owner->m_nClassIdxKeyEvent, refEvent);
		if (!refSelected) {
			// There can't be pressed keys without an active window
			break; // for -----------
		}
	}
}
void GtkXKeyboardDevice::removingDevice() noexcept
{
	resetOwnerDeviceManager();
}
void GtkXKeyboardDevice::cancelSelectedAccessorKeys() noexcept
{
	auto refOwner = getOwnerDeviceManager();
	if (!refOwner) {
		return;
	}
	FloGtkDeviceManager* p0Owner = refOwner.get();
	if (!p0Owner->isEventClassEnabled(Event::Class{typeid(KeyEvent)})) {
		return; // -------------------------------------------------------------
	}
	auto& refSelected = p0Owner->m_refSelected;
	if (!refSelected) {
		return; // -------------------------------------------------------------
	}
	auto refSelectedAccessor = refSelected->getAccessor();

	auto refListeners = p0Owner->getListeners();
	//
	shared_ptr<GtkXKeyboardDevice> refGtkXKeyboardDevice = shared_from_this();
	shared_ptr<KeyCapability> refCapability = refGtkXKeyboardDevice;
	// Remove all keys generated by the accessor (widget)
	const int64_t nEventTimeUsec = DeviceManager::getNowTimeMicroseconds();
	// work on copy
	auto oPressedKeys = m_oPressedKeys;
	for (auto& oPair : oPressedKeys) {
		const int32_t nHardwareKey = oPair.first;
		const KeyData& oKeyData = oPair.second;
		//
		const HARDWARE_KEY eHardwareKey = static_cast<HARDWARE_KEY>(nHardwareKey);
		const auto nKeyPressedTimeStamp = oKeyData.m_nPressedTimeStamp;
		//
		shared_ptr<Event> refEvent;
		for (auto& p0ListenerData : *refListeners) {
			FloGtkListenerExtraData* p0ExtraData = nullptr;
			p0ListenerData->getExtraData(p0ExtraData);
			if (p0ExtraData->isKeyCanceled(nHardwareKey)) {
				continue; // for itListenerData ------------
			}
			p0ExtraData->setKeyCanceled(nHardwareKey);
			//
			sendKeyEventToListener(*p0ListenerData, nEventTimeUsec, nKeyPressedTimeStamp, KeyEvent::KEY_RELEASE_CANCEL
									, eHardwareKey, refSelectedAccessor, refCapability, p0Owner->m_nClassIdxKeyEvent, refEvent);
		}
	}
	m_oPressedKeys.clear();
}
void GtkXKeyboardDevice::sendKeyEventToListener(const FloGtkDeviceManager::ListenerData& oListenerData, int64_t nEventTimeUsec
												, uint64_t nPressedTimeStamp
												, KeyEvent::KEY_INPUT_TYPE eInputType, HARDWARE_KEY eHardwareKey
												, const shared_ptr<GtkAccessor>& refAccessor
												, const shared_ptr<KeyCapability>& refCapability
												, int32_t nClassIdxKeyEvent
												, shared_ptr<Event>& refEvent) noexcept
{
	const auto nAddTimeStamp = oListenerData.getAddedTimeStamp();
//std::cout << "GtkXKeyboardDevice::sendKeyEventToListener nAddTimeUsec=" << nAddTimeUsec;
//std::cout << "  nTimePressedUsec=" << nTimePressedUsec << "  nEventTimeUsec=" << nEventTimeUsec << '\n';
	if (nPressedTimeStamp < nAddTimeStamp) {
		// The listener was added after the key was pressed
		return;
	}
	if (!refEvent) {
		m_oKeyEventRecycler.create(refEvent, nEventTimeUsec, refAccessor, refCapability, eInputType, eHardwareKey);
	}
	oListenerData.handleEventCallIf(nClassIdxKeyEvent, refEvent);
		// no need to reset because KeyEvent cannot be modified.
}

} // namespace Flo
} // namespace Private

} // namespace stmi
