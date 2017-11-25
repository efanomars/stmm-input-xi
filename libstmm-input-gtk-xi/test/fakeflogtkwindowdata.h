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
 * File:   fakeflogtkwindowdata.h
 */

#ifndef STMI_TESTING_FAKE_FLO_GTK_WINDOW_DATA_H
#define STMI_TESTING_FAKE_FLO_GTK_WINDOW_DATA_H

#include "flogtkwindowdata.h"

#include "fakeflogtkbackend.h"

namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

namespace testing
{
namespace Flo
{

class FakeGtkWindowDataFactory;

////////////////////////////////////////////////////////////////////////////////
class FakeGtkWindowData : public Private::Flo::GtkWindowData
{
public:
	FakeGtkWindowData(FakeGtkBackend* p0FakeGtkBackend)
	: Private::Flo::GtkWindowData()
	, m_p0FakeGtkBackend(p0FakeGtkBackend)
	, m_p0Factory(nullptr)
	, m_bIsRealized(false)
	, m_nXWinId(None)
	, m_bIsEnabled(false)
	, m_bWindowActive(false)
	{
		assert(p0FakeGtkBackend != nullptr);
	}
	void enable(const shared_ptr<GtkAccessor>& refAccessor, FloGtkDeviceManager* p0Owner) override;
	// !!! Doesn't reset accessor!
	void disable() override
	{
		m_bIsEnabled = false;
		disconnect();
		m_bIsRealized = false;
	}
	bool isEnabled() const override
	{
		return m_bIsEnabled;
	}
	void connectAllDevices() override
	{
		if (!m_bIsRealized) {
			return;
		}
		const auto aXDeviceIds = m_p0FakeGtkBackend->getXDeviceIds();
		for (auto nXDeviceId : aXDeviceIds) {
			connectDeviceToWindow(nXDeviceId);
		}
		assert(aXDeviceIds.size() == m_aConnectedDevices.size());
	}
	void connectDevice(int32_t nXDeviceId) override
	{
		if (!m_bIsRealized) {
			return;
		}
		const auto aXDeviceIds = m_p0FakeGtkBackend->getXDeviceIds();
		if (aXDeviceIds.empty()) {
			connectAllDevices();
		} else {
			#ifndef NDEBUG
			auto itFindExist = std::find(aXDeviceIds.begin(), aXDeviceIds.end(), nXDeviceId);
			#endif
			assert(itFindExist != aXDeviceIds.end());
			connectDeviceToWindow(nXDeviceId);
		}
	}
	const shared_ptr<GtkAccessor>& getAccessor() override
	{
		return m_refAccessor;
	}
	bool isWindowActive() const override
	{
		return m_bWindowActive;
	}
	::Window getXWindow() const override
	{
		if (m_bIsRealized) {
			return m_nXWinId;
		} else {
			return None;
		}
	}

	//
	void simulateWindowSetActive(bool bActive)
	{
		if (bActive) {
			simulateWindowRealized();
		}
		if (m_bWindowActive == bActive) {
			// no change
			return;
		}
		m_bWindowActive = bActive;
		onSigIsActiveChanged();
	}
	void simulateWindowRealized()
	{
		if (!m_bIsRealized) {
			m_nXWinId = getNewWinId();
			m_bIsRealized = true;
		} else {
			assert(m_nXWinId > 0);
		}
	}

private:
	static ::Window getNewWinId()
	{
		static ::Window s_nUniqueWindow = 0;
		++s_nUniqueWindow;
		return s_nUniqueWindow;
	}
	void disconnect()
	{
//std::cout << "GtkWindowData::disconnect()" << std::endl;
		m_aConnectedDevices.clear();
	}
	void connectDeviceToWindow(int32_t nXDeviceId)
	{
		auto itFind = std::find(m_aConnectedDevices.begin(), m_aConnectedDevices.end(), nXDeviceId);
		if (itFind == m_aConnectedDevices.end()) {
			m_aConnectedDevices.push_back(nXDeviceId);
		}
	}
	
private:
	FakeGtkBackend* m_p0FakeGtkBackend;
	friend class FakeGtkWindowDataFactory;
	FakeGtkWindowDataFactory* m_p0Factory;
	shared_ptr<GtkAccessor> m_refAccessor;
	bool m_bIsRealized;
	::Window m_nXWinId;
	//
	std::vector<int32_t> m_aConnectedDevices;
	//
	bool m_bIsEnabled;
	//
	bool m_bWindowActive;
};

////////////////////////////////////////////////////////////////////////////////
class FakeGtkWindowDataFactory : public Private::Flo::GtkWindowDataFactory
{
public:
	FakeGtkWindowDataFactory(FakeGtkBackend* p0FakeGtkBackend)
	: Private::Flo::GtkWindowDataFactory()
	, m_p0FakeGtkBackend(p0FakeGtkBackend)
	{
		assert(p0FakeGtkBackend != nullptr);
	}
	std::shared_ptr<Private::Flo::GtkWindowData> create() override
	{
		for (auto& refGtkWindowData : m_aFreePool) {
			if (refGtkWindowData.use_count() == 1) {
				// Recycle
				return refGtkWindowData; //-------------------------------------
			}
		}
		// The data is left in the pool!
		m_aFreePool.emplace_back(std::make_shared<FakeGtkWindowData>(m_p0FakeGtkBackend));
		auto& refNew = m_aFreePool.back();
		refNew->m_p0Factory = this;
		return refNew;
	}
	const std::shared_ptr<FakeGtkWindowData>& getFakeWindowData(Gtk::Window* p0GtkmmWindow)
	{
		for (auto& refGtkWindowData : m_aFreePool) {
			if (!(refGtkWindowData.use_count() == 1)) {
				auto& refAccessor = refGtkWindowData->getAccessor();
				if ((refAccessor) && (!refAccessor->isDeleted()) && (refAccessor->getGtkmmWindow() == p0GtkmmWindow)) {
					return refGtkWindowData;
				}
			}
		}
		static std::shared_ptr<FakeGtkWindowData> s_refEmpty{};
		return s_refEmpty;
	}
	// There should be max one active window at any time
	// Any way this function returns the first active found.
	const std::shared_ptr<FakeGtkWindowData>& getActiveWindowData()
	{
		for (auto& refGtkWindowData : m_aFreePool) {
			if (!(refGtkWindowData.use_count() == 1)) {
				auto& refAccessor = refGtkWindowData->getAccessor();
				if ((refAccessor) && (!refAccessor->isDeleted()) && refGtkWindowData->isWindowActive()) {
					return refGtkWindowData;
				}
			}
		}
		static std::shared_ptr<FakeGtkWindowData> s_refEmpty{};
		return s_refEmpty;
	}
	void simulateWindowSetRealizedWhenEnabled(bool bRealizedOnEnable)
	{
		m_bRealizedOnEnable = bRealizedOnEnable;
	}
private:
	FakeGtkBackend* m_p0FakeGtkBackend;
	// The objects in the free pool might still be in use when the
	// removal of the accessor was done during a callback. This is detected
	// through the ref count of the shared_ptr.
	std::vector< std::shared_ptr<FakeGtkWindowData> > m_aFreePool;
	//
	friend class FakeGtkWindowData;
	bool m_bRealizedOnEnable;
private:
	FakeGtkWindowDataFactory() = delete;
};

} // namespace Flo
} // namespace testing

} // namespace stmi

#endif /* STMI_TESTING_FAKE_FLO_GTK_WINDOW_DATA_H */
