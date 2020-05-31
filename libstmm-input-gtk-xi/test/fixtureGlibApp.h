/*
 * Copyright © 2016-2018  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   fixtureGlibApp.h
 */

#ifndef STMI_TESTING_FIXTURE_GLIB_APP_H
#define STMI_TESTING_FIXTURE_GLIB_APP_H

#include "fixtureTestBase.h"

#include <gtkmm.h>

namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

namespace testing
{

class GlibAppFixture : public TestBaseFixture
{
protected:
	void setup() override
	{
		TestBaseFixture::setup();
		m_refApp = Gtk::Application::create("net.testlibsnirvana.stmi");
	}
protected:
	Glib::RefPtr<Gtk::Application> m_refApp;
};

} // namespace testing

} // namespace stmi

#endif /* STMI_TESTING_FIXTURE_GLIB_APP_H */
