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
 * File:   testFloGtkDeviceManager.cc
 */

#include "fixtureFloDM.h"

#include <stmm-input/callifs.h>

#include <gtest/gtest.h>

namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

namespace testing
{

////////////////////////////////////////////////////////////////////////////////
TEST_F(GlibAppFixture, CreateFakeFloGtkDeviceManager)
{
	auto refFloDM = FakeFloGtkDeviceManager::create(false, {}, KeyRepeat::MODE_SUPPRESS
												, shared_ptr<GdkKeyConverter>{}, Glib::RefPtr<Gdk::Display>{});
	EXPECT_TRUE(refFloDM.operator bool());
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(FloDMFixture, AddAccessor)
{
	auto refWin1 = Glib::RefPtr<Gtk::Window>(new Gtk::Window());
	EXPECT_TRUE(refWin1.operator bool());
	auto refAccessor = std::make_shared<stmi::GtkAccessor>(refWin1);
	const bool bAdded = m_refAllEvDM->addAccessor(refAccessor);

	EXPECT_TRUE(bAdded);
	EXPECT_TRUE(m_refAllEvDM->hasAccessor(refAccessor));
	const bool bRemoved = m_refAllEvDM->removeAccessor(refAccessor);
	EXPECT_TRUE(bRemoved);
	EXPECT_FALSE(m_refAllEvDM->hasAccessor(refAccessor));
}

TEST_F(FloDMFixture, AddListener)
{
	auto refListener = std::make_shared<stmi::EventListener>(
			[](const shared_ptr<stmi::Event>& /*refEvent*/)
			{
			});
	const bool bAdded = m_refAllEvDM->addEventListener(refListener);
	EXPECT_TRUE(bAdded);
	const bool bAdded2 = m_refAllEvDM->addEventListener(refListener);
	EXPECT_FALSE(bAdded2);

	const bool bRemoved = m_refAllEvDM->removeEventListener(refListener);
	EXPECT_TRUE(bRemoved);
	const bool bRemoved2 = m_refAllEvDM->removeEventListener(refListener);
	EXPECT_FALSE(bRemoved2);
}

TEST_F(FloDMFixture, DeviceAddAndRemoveChange)
{
	std::vector< shared_ptr<stmi::Event> > aReceivedEvents;
	auto refListener = std::make_shared<stmi::EventListener>(
			[&](const shared_ptr<stmi::Event>& refEvent)
			{
				aReceivedEvents.emplace_back(refEvent);
			});
	const bool bListenerAdded = m_refAllEvDM->addEventListener(refListener, std::shared_ptr<stmi::CallIf>{});
	EXPECT_TRUE(bListenerAdded);

	auto refFakeBackend = m_refAllEvDM->getBackend();

	const int32_t nSimuId1 = refFakeBackend->simulateNewDevice();
	EXPECT_TRUE(nSimuId1 >= 0);
	EXPECT_TRUE(aReceivedEvents.size() == 1);
	int32_t nDeviceId1;
	{
		const shared_ptr<stmi::Event>& refEvent = aReceivedEvents[0];
		EXPECT_TRUE(refEvent.operator bool());
		nDeviceId1 = refEvent->getCapabilityId();
		EXPECT_TRUE(refEvent->getEventClass() == typeid(stmi::DeviceMgmtEvent));
		auto refDeviceMgmtEvent = std::static_pointer_cast<stmi::DeviceMgmtEvent>(refEvent);
		EXPECT_TRUE(refDeviceMgmtEvent->getDeviceMgmtType() == stmi::DeviceMgmtEvent::DEVICE_MGMT_ADDED);
	}

	const int32_t nSimuId2 = refFakeBackend->simulateNewDevice();
	EXPECT_TRUE(nSimuId2 >= 0);
	EXPECT_TRUE(aReceivedEvents.size() == 2);
	int32_t nDeviceId2;
	{
		const shared_ptr<stmi::Event>& refEvent = aReceivedEvents[1];
		EXPECT_TRUE(refEvent.operator bool());
		nDeviceId2 = refEvent->getCapabilityId();
		EXPECT_TRUE(refEvent->getEventClass() == typeid(stmi::DeviceMgmtEvent));
		auto refDeviceMgmtEvent = std::static_pointer_cast<stmi::DeviceMgmtEvent>(refEvent);
		EXPECT_TRUE(refDeviceMgmtEvent->getDeviceMgmtType() == stmi::DeviceMgmtEvent::DEVICE_MGMT_ADDED);
	}

	refFakeBackend->simulateRemoveDevice(nSimuId1);
	EXPECT_TRUE(aReceivedEvents.size() == 3);
	{
		const shared_ptr<stmi::Event>& refEvent = aReceivedEvents[2];
		EXPECT_TRUE(refEvent.operator bool());
		EXPECT_TRUE(refEvent->getCapabilityId() == nDeviceId1);
		EXPECT_TRUE(refEvent->getEventClass() == typeid(stmi::DeviceMgmtEvent));
		auto refDeviceMgmtEvent = std::static_pointer_cast<stmi::DeviceMgmtEvent>(refEvent);
		EXPECT_TRUE(refDeviceMgmtEvent->getDeviceMgmtType() == stmi::DeviceMgmtEvent::DEVICE_MGMT_REMOVED);
		// when removed device persists only within callback
		EXPECT_FALSE(refDeviceMgmtEvent->getDevice().operator bool());
	}

	refFakeBackend->simulateDeviceChanged(nSimuId2);
	EXPECT_TRUE(aReceivedEvents.size() == 4);
	{
		const shared_ptr<stmi::Event>& refEvent = aReceivedEvents[3];
		EXPECT_TRUE(refEvent.operator bool());
		EXPECT_TRUE(refEvent->getCapabilityId() == nDeviceId2);
		EXPECT_TRUE(refEvent->getEventClass() == typeid(stmi::DeviceMgmtEvent));
		auto refDeviceMgmtEvent = std::static_pointer_cast<stmi::DeviceMgmtEvent>(refEvent);
		EXPECT_TRUE(refDeviceMgmtEvent->getDeviceMgmtType() == stmi::DeviceMgmtEvent::DEVICE_MGMT_CHANGED);
	}

	refFakeBackend->simulateRemoveDevice(nSimuId2);
	EXPECT_TRUE(aReceivedEvents.size() == 5);
	{
		const shared_ptr<stmi::Event>& refEvent = aReceivedEvents[4];
		EXPECT_TRUE(refEvent.operator bool());
		EXPECT_TRUE(refEvent->getCapabilityId() == nDeviceId2);
		EXPECT_TRUE(refEvent->getEventClass() == typeid(stmi::DeviceMgmtEvent));
		auto refDeviceMgmtEvent = std::static_pointer_cast<stmi::DeviceMgmtEvent>(refEvent);
		EXPECT_TRUE(refDeviceMgmtEvent->getDeviceMgmtType() == stmi::DeviceMgmtEvent::DEVICE_MGMT_REMOVED);
		// when removed device persists only within callback
		EXPECT_FALSE(refDeviceMgmtEvent->getDevice().operator bool());
	}
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(FloDMOneWinOneAccOneDevOneListenerFixture, SingleKeyPress)
{
	auto refFakeFactory = m_refAllEvDM->getFactory();
	const std::shared_ptr<Flo::FakeGtkWindowData>& refWinData1 = refFakeFactory->getFakeWindowData(m_refWin1.operator->());
	EXPECT_TRUE(refWinData1.operator bool());
	assert(!refWinData1->isWindowActive());

	m_refAllEvDM->makeWindowActive(m_refGtkAccessor1);
	assert(refWinData1->isWindowActive());

	EXPECT_TRUE(m_aReceivedEvents1.size() == 0);

	m_p0FakeBackend->simulateKeyEvent(m_nXDeviceId, refWinData1->getXWindow(), true, 67); // F1 evdev key

	EXPECT_TRUE(m_aReceivedEvents1.size() == 1);

	const shared_ptr<stmi::Event>& refEvent = m_aReceivedEvents1[0];
	EXPECT_TRUE(refEvent.operator bool());
	EXPECT_TRUE(refEvent->getEventClass() == typeid(stmi::KeyEvent));
	auto p0KeyEvent = static_cast<stmi::KeyEvent*>(refEvent.get());
	EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F1);
	EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_PRESS);
}

TEST_F(FloDMOneWinOneAccOneDevOneListenerFixture, TwoAccessorKeyCancel)
{
	auto refWin2 = Glib::RefPtr<Gtk::Window>(new Gtk::Window());
	assert(refWin2.operator bool());
	auto refGtkAccessor2 = std::make_shared<stmi::GtkAccessor>(refWin2);
	assert(refGtkAccessor2.operator bool());
	#ifndef NDEBUG
	const bool bAdded = 
	#endif
	m_refAllEvDM->addAccessor(refGtkAccessor2);
	assert(bAdded);

	auto refFakeFactory = m_refAllEvDM->getFactory();
	const std::shared_ptr<Flo::FakeGtkWindowData>& refWinData1 = refFakeFactory->getFakeWindowData(m_refWin1.operator->());
	EXPECT_TRUE(refWinData1.operator bool());
	const std::shared_ptr<Flo::FakeGtkWindowData>& refWinData2 = refFakeFactory->getFakeWindowData(refWin2.operator->());
	EXPECT_TRUE(refWinData2.operator bool());

	EXPECT_TRUE(m_refAllEvDM->hasAccessor(m_refGtkAccessor1));
	EXPECT_TRUE(m_refAllEvDM->hasAccessor(refGtkAccessor2));

	m_refAllEvDM->makeWindowActive(m_refGtkAccessor1);

	m_p0FakeBackend->simulateKeyEvent(m_nXDeviceId, refWinData1->getXWindow(), true, 67); // press F1 evdev key

	m_refAllEvDM->makeWindowActive(refGtkAccessor2);

	m_p0FakeBackend->simulateKeyEvent(m_nXDeviceId, refWinData2->getXWindow(), true, 68); // press F2 evdev key

	m_refAllEvDM->removeAccessor(refGtkAccessor2);

	EXPECT_TRUE(m_refAllEvDM->hasAccessor(m_refGtkAccessor1));
	EXPECT_FALSE(m_refAllEvDM->hasAccessor(refGtkAccessor2));

	EXPECT_TRUE(m_aReceivedEvents1.size() == 4); // received both F1 and F2 Press + Cancel to active window

	for (auto nIdx = 0u; nIdx < m_aReceivedEvents1.size(); ++nIdx) {
		auto& refEvent = m_aReceivedEvents1[nIdx];
		EXPECT_TRUE(refEvent.operator bool());
		EXPECT_TRUE(refEvent->getEventClass() == typeid(stmi::KeyEvent));
		auto p0KeyEvent = static_cast<stmi::KeyEvent*>(refEvent.get());
		if (nIdx == 0) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F1);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_PRESS);
		} else if (nIdx == 1) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F1);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_RELEASE_CANCEL);
		} else if (nIdx == 2) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F2);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_PRESS);
		} else if (nIdx == 3) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F2);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_RELEASE_CANCEL);
		}
	}
}

TEST_F(FloDMOneWinOneAccOneDevOneListenerFixture, ThreeListeners)
{
	m_refAllEvDM->makeWindowActive(m_refGtkAccessor1);

	std::vector<shared_ptr<stmi::Event> > aReceivedEvents3;
	auto refListener3 = std::make_shared<stmi::EventListener>(
			[&](const shared_ptr<stmi::Event>& refEvent)
			{
				aReceivedEvents3.emplace_back(refEvent);
			});
	//
	std::vector<shared_ptr<stmi::Event> > aReceivedEvents2;
	auto refListener2 = std::make_shared<stmi::EventListener>(
			[&](const shared_ptr<stmi::Event>& refEvent)
			{
				aReceivedEvents2.emplace_back(refEvent);
				EXPECT_TRUE(refEvent.operator bool());
				EXPECT_TRUE(refEvent->getEventClass() == typeid(stmi::KeyEvent));
				auto p0KeyEvent = static_cast<stmi::KeyEvent*>(refEvent.get());
				if ((p0KeyEvent->getType() == KeyEvent::KEY_PRESS) && (p0KeyEvent->getKey() == stmi::HK_F1)) {
					// Add third listener
					const bool bListenerAdded3 = m_refAllEvDM->addEventListener(refListener3, std::shared_ptr<stmi::CallIf>{});
					EXPECT_TRUE(bListenerAdded3);
				}
			});
	const bool bListenerAdded2 = m_refAllEvDM->addEventListener(refListener2, std::make_shared<stmi::CallIfEventClass>(stmi::KeyEvent::getClass()));
	EXPECT_TRUE(bListenerAdded2);

	auto refFakeFactory = m_refAllEvDM->getFactory();
	const std::shared_ptr<Flo::FakeGtkWindowData>& refWinData1 = refFakeFactory->getFakeWindowData(m_refWin1.operator->());
	assert(refWinData1);

	m_p0FakeBackend->simulateKeyEvent(m_nXDeviceId, refWinData1->getXWindow(), true, 67); // press F1 evdev key
	
	EXPECT_TRUE(m_aReceivedEvents1.size() == 1);
	EXPECT_TRUE(aReceivedEvents2.size() == 1);
	EXPECT_TRUE(aReceivedEvents3.size() == 0);

	m_p0FakeBackend->simulateKeyEvent(m_nXDeviceId, refWinData1->getXWindow(), false, 67); // release F1 evdev key

	EXPECT_TRUE(m_aReceivedEvents1.size() == 2);
	EXPECT_TRUE(aReceivedEvents2.size() == 2);
	EXPECT_TRUE(aReceivedEvents3.size() == 0); // Since it wasn't pressed when added listener 3 doesn't receive release

	m_p0FakeBackend->simulateKeyEvent(m_nXDeviceId, refWinData1->getXWindow(), true, 68); // press F2 evdev key

	EXPECT_TRUE(m_aReceivedEvents1.size() == 3);
	EXPECT_TRUE(aReceivedEvents2.size() == 3);
	EXPECT_TRUE(aReceivedEvents3.size() == 1);

	m_refAllEvDM->removeEventListener(m_refListener1, true);

	EXPECT_TRUE(m_aReceivedEvents1.size() == 4);
	EXPECT_TRUE(aReceivedEvents2.size() == 3);
	EXPECT_TRUE(aReceivedEvents3.size() == 1);

	m_p0FakeBackend->simulateKeyEvent(m_nXDeviceId, refWinData1->getXWindow(), true, 68); // press F2 evdev key (suppressed)

	EXPECT_TRUE(m_aReceivedEvents1.size() == 4);
	EXPECT_TRUE(aReceivedEvents2.size() == 3);
	EXPECT_TRUE(aReceivedEvents3.size() == 1);

	m_p0FakeBackend->simulateKeyEvent(m_nXDeviceId, refWinData1->getXWindow(), true, 69); // press F3 evdev key

	EXPECT_TRUE(m_aReceivedEvents1.size() == 4); // no longer active
	EXPECT_TRUE(aReceivedEvents2.size() == 4);
	EXPECT_TRUE(aReceivedEvents3.size() == 2);

	const bool bRemoved = m_refAllEvDM->removeAccessor(m_refGtkAccessor1);
	EXPECT_TRUE(bRemoved);

	EXPECT_TRUE(m_aReceivedEvents1.size() == 4); // no longer active
	EXPECT_TRUE(aReceivedEvents2.size() == 6);
	EXPECT_TRUE(aReceivedEvents3.size() == 4);

	for (auto nIdx = 0u; nIdx < m_aReceivedEvents1.size(); ++nIdx) {
		auto& refEvent = m_aReceivedEvents1[nIdx];
		EXPECT_TRUE(refEvent.operator bool());
		EXPECT_TRUE(refEvent->getEventClass() == typeid(stmi::KeyEvent));
		auto p0KeyEvent = static_cast<stmi::KeyEvent*>(refEvent.get());
		if (nIdx == 0) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F1);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_PRESS);
		} else if (nIdx == 1) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F1);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_RELEASE);
		} else if (nIdx == 2) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F2);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_PRESS);
		} else if (nIdx == 3) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F2);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_RELEASE_CANCEL);
		}
	}
	{
		stmi::HARDWARE_KEY eKey = stmi::HK_NULL;
		stmi::Event::AS_KEY_INPUT_TYPE eType = stmi::Event::AS_KEY_PRESS;
		bool bMoreThanOne = true;
		const bool bAsKey = aReceivedEvents2[4]->getAsKey(eKey, eType, bMoreThanOne);
		EXPECT_TRUE(bAsKey);
		EXPECT_FALSE(bMoreThanOne);
		if (eKey == stmi::HK_F3) {
			// the order of cancels sent when an accessor is removed is non-deterministic
			std::swap(aReceivedEvents2[4], aReceivedEvents2[5]);
		}
	}
	for (auto nIdx = 0u; nIdx < aReceivedEvents2.size(); ++nIdx) {
		auto& refEvent = aReceivedEvents2[nIdx];
		auto p0KeyEvent = static_cast<stmi::KeyEvent*>(refEvent.get());
		if (nIdx == 0) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F1);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_PRESS);
		} else if (nIdx == 1) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F1);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_RELEASE);
		} else if (nIdx == 2) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F2);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_PRESS);
		} else if (nIdx == 3) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F3);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_PRESS);
		} else if (nIdx == 4) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F2);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_RELEASE_CANCEL);
		} else if (nIdx == 5) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F3);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_RELEASE_CANCEL);
		}
	}
	{
		stmi::HARDWARE_KEY eKey = stmi::HK_NULL;
		stmi::Event::AS_KEY_INPUT_TYPE eType = stmi::Event::AS_KEY_PRESS;
		bool bMoreThanOne = true;
		const bool bAsKey = aReceivedEvents3[2]->getAsKey(eKey, eType, bMoreThanOne);
		EXPECT_TRUE(bAsKey);
		EXPECT_FALSE(bMoreThanOne);
		if (eKey == stmi::HK_F3) {
			// the order of cancels sent when an accessor is removed is non-deterministic
			std::swap(aReceivedEvents3[2], aReceivedEvents3[3]);
		}
	}
	for (auto nIdx = 0u; nIdx < aReceivedEvents3.size(); ++nIdx) {
		auto& refEvent = aReceivedEvents3[nIdx];
		auto p0KeyEvent = static_cast<stmi::KeyEvent*>(refEvent.get());
		if (nIdx == 0) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F2);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_PRESS);
		} else if (nIdx == 1) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F3);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_PRESS);
		} else if (nIdx == 2) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F2);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_RELEASE_CANCEL);
		} else if (nIdx == 3) {
			EXPECT_TRUE(p0KeyEvent->getKey() == stmi::HK_F3);
			EXPECT_TRUE(p0KeyEvent->getType() == stmi::KeyEvent::KEY_RELEASE_CANCEL);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
class FloDMAddReleaseOneWinOneAccOneDevOneListenerFixture : public FloDMOneWinOneAccOneDevOneListenerFixture
																, public FixtureVariantKeyRepeatMode_AddRelease
{
};
TEST_F(FloDMAddReleaseOneWinOneAccOneDevOneListenerFixture, DoubleKeyPressAddRelease)
{
	auto refFakeFactory = m_refAllEvDM->getFactory();
	const std::shared_ptr<Flo::FakeGtkWindowData>& refWinData1 = refFakeFactory->getFakeWindowData(m_refWin1.operator->());
	EXPECT_TRUE(refWinData1.operator bool());
	assert(!refWinData1->isWindowActive());

	m_refAllEvDM->makeWindowActive(m_refGtkAccessor1);
	assert(refWinData1->isWindowActive());

	EXPECT_TRUE(m_aReceivedEvents1.size() == 0);

	m_p0FakeBackend->simulateKeyEvent(m_nXDeviceId, refWinData1->getXWindow(), true, 67); // F1 evdev key

	EXPECT_TRUE(m_aReceivedEvents1.size() == 1);

	m_p0FakeBackend->simulateKeyEvent(m_nXDeviceId, refWinData1->getXWindow(), true, 67); // F1 evdev key

	EXPECT_TRUE(m_aReceivedEvents1.size() == 3);

	const shared_ptr<stmi::Event>& refEvent1 = m_aReceivedEvents1[0];
	EXPECT_TRUE(refEvent1.operator bool());
	EXPECT_TRUE(refEvent1->getEventClass() == typeid(stmi::KeyEvent));
	auto p0KeyEvent1 = static_cast<stmi::KeyEvent*>(refEvent1.get());
	EXPECT_TRUE(p0KeyEvent1->getKey() == stmi::HK_F1);
	EXPECT_TRUE(p0KeyEvent1->getType() == stmi::KeyEvent::KEY_PRESS);

	const shared_ptr<stmi::Event>& refEvent2 = m_aReceivedEvents1[1];
	EXPECT_TRUE(refEvent2.operator bool());
	EXPECT_TRUE(refEvent2->getEventClass() == typeid(stmi::KeyEvent));
	auto p0KeyEvent2 = static_cast<stmi::KeyEvent*>(refEvent2.get());
	EXPECT_TRUE(p0KeyEvent2->getKey() == stmi::HK_F1);
	EXPECT_TRUE(p0KeyEvent2->getType() == stmi::KeyEvent::KEY_RELEASE);

	const shared_ptr<stmi::Event>& refEvent3 = m_aReceivedEvents1[2];
	EXPECT_TRUE(refEvent3.operator bool());
	EXPECT_TRUE(refEvent3->getEventClass() == typeid(stmi::KeyEvent));
	auto p0KeyEvent3 = static_cast<stmi::KeyEvent*>(refEvent3.get());
	EXPECT_TRUE(p0KeyEvent3->getKey() == stmi::HK_F1);
	EXPECT_TRUE(p0KeyEvent3->getType() == stmi::KeyEvent::KEY_PRESS);
}

////////////////////////////////////////////////////////////////////////////////
class FloDMDelayedEventEnablingFixture : public FloDMOneWinOneAccOneDevOneListenerFixture
										, public FixtureVariantEventClassesEnable_True
										, public FixtureVariantEventClasses_DeviceMgmtEvent
{
};
TEST_F(FloDMDelayedEventEnablingFixture, DelayedEventEnabling)
{
	EXPECT_TRUE(m_refAllEvDM->isEventClassEnabled(DeviceMgmtEvent::getClass()));
	EXPECT_FALSE(m_refAllEvDM->isEventClassEnabled(KeyEvent::getClass()));

	auto refFakeFactory = m_refAllEvDM->getFactory();
	const std::shared_ptr<Flo::FakeGtkWindowData>& refWinData1 = refFakeFactory->getFakeWindowData(m_refWin1.operator->());
	EXPECT_TRUE(refWinData1.operator bool());
	assert(!refWinData1->isWindowActive());

	m_refAllEvDM->makeWindowActive(m_refGtkAccessor1);
	assert(refWinData1->isWindowActive());

	auto nXDeviceId = m_p0FakeBackend->simulateNewDevice();
	assert(nXDeviceId >= 0);

	EXPECT_TRUE(m_aReceivedEvents1.size() == 1);

	m_refAllEvDM->enableEventClass(KeyEvent::getClass());

	EXPECT_TRUE(m_refAllEvDM->isEventClassEnabled(DeviceMgmtEvent::getClass()));
	EXPECT_TRUE(m_refAllEvDM->isEventClassEnabled(KeyEvent::getClass()));

	m_p0FakeBackend->simulateKeyEvent(nXDeviceId, refWinData1->getXWindow(), true, 67); // press F1 evdev key

	EXPECT_TRUE(m_aReceivedEvents1.size() == 2);
}

} // namespace testing

} // namespace stmi
