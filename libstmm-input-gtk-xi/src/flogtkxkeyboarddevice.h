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
 * File:   flogtkxkeyboarddevice.h
 */

#ifndef STMI_FLO_GTK_X_KEYBOARD_DEVICE_H
#define STMI_FLO_GTK_X_KEYBOARD_DEVICE_H

#include "flogtkdevicemanager.h"

#include "recycler.h"

#include <stmm-input-base/basicdevice.h>

namespace stmi
{

namespace Private
{
namespace Flo
{

using std::shared_ptr;
using std::weak_ptr;

class GtkXKeyboardDevice final : public BasicDevice<FloGtkDeviceManager>, public KeyCapability
								, public std::enable_shared_from_this<GtkXKeyboardDevice>
{
public:
	GtkXKeyboardDevice(std::string sName, const shared_ptr<FloGtkDeviceManager>& refFloGtkDeviceManager)
	: BasicDevice<FloGtkDeviceManager>(sName, refFloGtkDeviceManager)
	{
	}
	virtual ~GtkXKeyboardDevice();
	shared_ptr<Capability> getCapability(const Capability::Class& oClass) const override;
	shared_ptr<Capability> getCapability(int32_t nCapabilityId) const override;
	std::vector<int32_t> getCapabilities() const override;
	std::vector<Capability::Class> getCapabilityClasses() const override;
	//
	shared_ptr<Device> getDevice() const override;
	bool isKeyboard() const override;
private:
	friend class stmi::FloGtkDeviceManager;
	void cancelSelectedAccessorKeys();
	void finalizeListener(FloGtkDeviceManager::ListenerData& oListenerData, int64_t nEventTimeUsec);
	void removingDevice();

	bool handleXIDeviceEvent(XIDeviceEvent* p0XIDeviceEvent, const shared_ptr<Private::Flo::GtkWindowData>& refWindowData);
	void sendKeyEventToListener(const FloGtkDeviceManager::ListenerData& oListenerData, int64_t nEventTimeUsec
								, uint64_t nPressedTimeStamp
								, KeyEvent::KEY_INPUT_TYPE eInputType, HARDWARE_KEY eHardwareKey
								, const shared_ptr<GtkAccessor>& refAccessor
								, const shared_ptr<KeyCapability>& refCapability
								, int32_t nClassIdxKeyEvent
								, shared_ptr<Event>& refEvent);
private:
	struct KeyData
	{
		uint64_t m_nPressedTimeStamp;
	};
	std::unordered_map<HARDWARE_KEY, KeyData> m_oPressedKeys;
	//
	class ReKeyEvent :public KeyEvent
	{
	public:
		ReKeyEvent(int64_t nTimeUsec, const shared_ptr<Accessor>& refAccessor
					, const shared_ptr<KeyCapability>& refKeyCapability, KEY_INPUT_TYPE eType, HARDWARE_KEY eKey)
		: KeyEvent(nTimeUsec, refAccessor, refKeyCapability, eType, eKey)
		{
		}
		void reInit(int64_t nTimeUsec, const shared_ptr<Accessor>& refAccessor
				, const shared_ptr<KeyCapability>& refKeyCapability, KEY_INPUT_TYPE eType, HARDWARE_KEY eKey)
		{
			setTimeUsec(nTimeUsec);
			setAccessor(refAccessor);
			setKeyCapability(refKeyCapability);
			setType(eType);
			setKey(eKey);
		}
	};
	Private::Recycler<ReKeyEvent, Event> m_oKeyEventRecycler;
private:
	GtkXKeyboardDevice(const GtkXKeyboardDevice& oSource) = delete;
	GtkXKeyboardDevice& operator=(const GtkXKeyboardDevice& oSource) = delete;
};

} // namespace Flo
} // namespace Private

} // namespace stmi

#endif /* STMI_FLO_GTK_X_KEYBOARD_DEVICE_H */
