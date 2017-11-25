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
 * File:   fixtureFloDM.h
 */

#ifndef STMI_TESTING_FIXTURE_FLO_DM_H
#define STMI_TESTING_FIXTURE_FLO_DM_H

#include "fixtureGlibApp.h"

#include "fixturevariantKeyRepeatMode.h"
#include "fixturevariantEventClasses.h"

#include "fakeflogtkdevicemanager.h"
#include <stmm-input/devicemanager.h>

namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

namespace testing
{

////////////////////////////////////////////////////////////////////////////////
class FloDMFixture : public GlibAppFixture
					, public FixtureVariantKeyRepeatMode
					, public FixtureVariantEventClassesEnable
					, public FixtureVariantEventClasses
{
protected:
	void SetUp() override
	{
		GlibAppFixture::SetUp();
		//
		const KeyRepeat::MODE eKeyRepeatMode = FixtureVariantKeyRepeatMode::getKeyRepeatMode();
		const bool bEventClassesEnable = FixtureVariantEventClassesEnable::getEnable();
		const std::vector<Event::Class> aClasses = FixtureVariantEventClasses::getEventClasses();
		//
		m_refAllEvDM = FakeFloGtkDeviceManager::create(bEventClassesEnable, aClasses, eKeyRepeatMode
													, shared_ptr<GdkKeyConverter>{}, Glib::RefPtr<Gdk::Display>{});
		assert(m_refAllEvDM.operator bool());
	}
	void TearDown() override
	{
		m_refAllEvDM.reset();
		GlibAppFixture::TearDown();
	}
public:
	shared_ptr<FakeFloGtkDeviceManager> m_refAllEvDM;
};

////////////////////////////////////////////////////////////////////////////////
class FloDMOneWinFixture : public FloDMFixture
{
protected:
	void SetUp() override
	{
		FloDMFixture::SetUp();
		m_refWin1 = Glib::RefPtr<Gtk::Window>(new Gtk::Window());
		assert(m_refWin1.operator bool());
	}
	void TearDown() override
	{
		m_refWin1.reset();
		FloDMFixture::TearDown();
	}
public:
	Glib::RefPtr<Gtk::Window> m_refWin1;
};

////////////////////////////////////////////////////////////////////////////////
class FloDMOneWinOneAccFixture : public FloDMOneWinFixture
{
protected:
	void SetUp() override
	{
		FloDMOneWinFixture::SetUp();
		m_refGtkAccessor1 = std::make_shared<stmi::GtkAccessor>(m_refWin1);
		assert(m_refGtkAccessor1.operator bool());
		#ifndef NDEBUG
		const bool bAdded = 
		#endif
		m_refAllEvDM->addAccessor(m_refGtkAccessor1);
		assert(bAdded);
	}
	void TearDown() override
	{
		m_refGtkAccessor1.reset();
		FloDMOneWinFixture::TearDown();
	}
public:
	shared_ptr<stmi::GtkAccessor> m_refGtkAccessor1;
};

////////////////////////////////////////////////////////////////////////////////
class FloDMOneWinOneAccOneDevFixture : public FloDMOneWinOneAccFixture
{
protected:
	void SetUp() override
	{
		FloDMOneWinOneAccFixture::SetUp();
		m_p0FakeBackend = m_refAllEvDM->getBackend();
		assert(m_p0FakeBackend != nullptr);
		m_nXDeviceId = m_p0FakeBackend->simulateNewDevice();
		assert(m_nXDeviceId >= 0);
	}
	void TearDown() override
	{
		m_nXDeviceId = -1;
		m_p0FakeBackend = nullptr;
		FloDMOneWinOneAccFixture::TearDown();
	}
public:
	Flo::FakeGtkBackend* m_p0FakeBackend;
	int32_t m_nXDeviceId;
};

////////////////////////////////////////////////////////////////////////////////
class FloDMOneWinOneAccOneDevOneListenerFixture : public FloDMOneWinOneAccOneDevFixture
{
protected:
	void SetUp() override
	{
		FloDMOneWinOneAccOneDevFixture::SetUp();
		m_refListener1 = std::make_shared<stmi::EventListener>(
				[&](const shared_ptr<stmi::Event>& refEvent)
				{
					m_aReceivedEvents1.emplace_back(refEvent);
				});
		#ifndef NDEBUG
		const bool bListenerAdded =
		#endif
		m_refAllEvDM->addEventListener(m_refListener1, std::shared_ptr<stmi::CallIf>{});
		assert(bListenerAdded);
	}
	void TearDown() override
	{
		m_refListener1.reset();
		m_aReceivedEvents1.clear();
		FloDMOneWinOneAccOneDevFixture::TearDown();
	}
public:
	std::vector< shared_ptr<stmi::Event> > m_aReceivedEvents1;
	shared_ptr<stmi::EventListener> m_refListener1;
};

} // namespace testing

} // namespace stmi

#endif /* STMI_TESTING_FIXTURE_FLO_DM_H */
