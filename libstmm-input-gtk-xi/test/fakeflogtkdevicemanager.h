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
 * File:   fakeflogtkdevicemanager.h
 */

#ifndef STMI_TESTING_FAKE_FLO_GTK_DEVICE_MANAGER_H
#define STMI_TESTING_FAKE_FLO_GTK_DEVICE_MANAGER_H

#include "flogtkdevicemanager.h"

#include <stmm-input-gtk/gdkkeyconverterevdev.h>

#include "fakeflogtkbackend.h"
#include "fakeflogtkwindowdata.h"

namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

namespace testing
{

class FakeFloGtkDeviceManager : public FloGtkDeviceManager
{
public:
	static shared_ptr<FakeFloGtkDeviceManager> create(bool bEnableEventClasses, const std::vector<Event::Class>& aEnDisableEventClasses
												, KeyRepeat::MODE eKeyRepeatMode, const shared_ptr<GdkKeyConverter>& refGdkConverter
												, const Glib::RefPtr<Gdk::Display>& refGdkDisplay) noexcept
	{
		shared_ptr<FakeFloGtkDeviceManager> refInstance(
				new FakeFloGtkDeviceManager(bEnableEventClasses, aEnDisableEventClasses
											, eKeyRepeatMode
											, (refGdkConverter ? refGdkConverter : GdkKeyConverterEvDev::getConverter())));
//std::cout << "FakeFloGtkDeviceManager::create 1" << '\n';
		auto refBackend = std::make_unique<Flo::FakeGtkBackend>(refInstance.operator->()
										, (refGdkDisplay ? refGdkDisplay : Gdk::Display::get_default()));
		auto refFactory = std::make_unique<Flo::FakeGtkWindowDataFactory>(refBackend.operator->());
		refInstance->init(refFactory, refBackend);
		return refInstance;
	}
	FakeFloGtkDeviceManager(bool bEnableEventClasses, const std::vector<Event::Class>& aEnDisableEventClasses
						, KeyRepeat::MODE eKeyRepeatMode, const shared_ptr<GdkKeyConverter>& refGdkConverter) noexcept
	: FloGtkDeviceManager(bEnableEventClasses, aEnDisableEventClasses, eKeyRepeatMode, refGdkConverter)
	, m_p0Factory(nullptr)
	, m_p0Backend(nullptr)
	{
//std::cout << "FakeFloGtkDeviceManager::FakeFloGtkDeviceManager" << '\n';
	}

	bool makeWindowActive(const shared_ptr<stmi::GtkAccessor>& refGtkAccessor) noexcept
	{
//std::cout << "FakeFloGtkDeviceManager::makeWindowActive 0" << '\n';
		assert(m_p0Factory != nullptr);
		assert(refGtkAccessor);
		auto p0GtkmmWindow = refGtkAccessor->getGtkmmWindow();
		if (refGtkAccessor->isDeleted()) {
			return false; //-----------------------------------------------------
		}
		auto& refOldWinData = m_p0Factory->getActiveWindowData();
		if (refOldWinData) {
			assert(refOldWinData->getAccessor());
			if (refOldWinData->getAccessor()->getGtkmmWindow() == p0GtkmmWindow) {
				// already active
				return true; //-----------------------------------------------------
			}
			// set old active to inactive
			refOldWinData->simulateWindowSetActive(false);
		}
		auto& refNewWinData = m_p0Factory->getFakeWindowData(p0GtkmmWindow);
		refNewWinData->simulateWindowSetActive(true);
		return true;
	}
	Flo::FakeGtkWindowDataFactory* getFactory() noexcept { return m_p0Factory; }
	Flo::FakeGtkBackend* getBackend() noexcept { return m_p0Backend; }
protected:
	void init(std::unique_ptr<Flo::FakeGtkWindowDataFactory>& refFactory
			, std::unique_ptr<Flo::FakeGtkBackend>& refBackend)
	{
		m_p0Factory = refFactory.get();
		m_p0Backend = refBackend.get();
		std::unique_ptr<Private::Flo::GtkWindowDataFactory> refF(refFactory.release());
		std::unique_ptr<Private::Flo::GtkBackend> refB(refBackend.release());
		assert(m_p0Factory == refF.get());
		assert(m_p0Backend == refB.get());
		FloGtkDeviceManager::init(refF, refB);
	}
private:
	Flo::FakeGtkWindowDataFactory* m_p0Factory;
	Flo::FakeGtkBackend* m_p0Backend;
};

} // namespace testing

} // namespace stmi

#endif /* STMI_TESTING_FAKE_FLO_GTK_DEVICE_MANAGER_H */
