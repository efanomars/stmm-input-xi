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
 * File:   fixtureGlibApp.h
 */

#ifndef STMI_TESTING_FIXTURE_GLIB_APP_H
#define STMI_TESTING_FIXTURE_GLIB_APP_H

#include <gtkmm.h>

#include <gtest/gtest.h>

namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

namespace testing
{

class GlibAppFixture : public ::testing::Test
{
protected:
	void SetUp() override
	{
		m_refApp = Gtk::Application::create("net.testlibsnirvana.stmi");
	}
	void TearDown() override
	{
		m_refApp.clear();
	}
public:
	Glib::RefPtr<Gtk::Application> m_refApp;
};

} // namespace testing

} // namespace stmi

#endif /* STMI_TESTING_FIXTURE_GLIB_APP_H */
