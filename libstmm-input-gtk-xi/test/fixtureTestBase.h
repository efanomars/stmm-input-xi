/*
 * Copyright © 2018-2019  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   fixtureTestBase.h
 */

#ifndef STMI_TESTING_FIXTURE_TEST_BASE_H
#define STMI_TESTING_FIXTURE_TEST_BASE_H

#include <type_traits>

namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

namespace testing
{

class TestBaseFixture
{
public:
	virtual ~TestBaseFixture() = default;
	TestBaseFixture() = default;
protected:
	virtual void setup() {}
	virtual void teardown() {}
};

template<class Base>
class STFX : public Base
{
public:
	STFX()
	{
		static_assert(std::is_base_of<TestBaseFixture, Base>::value, "");
		setup();
	}
	~STFX()
	{
		teardown();
	}
protected:
	void setup() override
	{
		Base::setup();
	}
	void teardown() override
	{
		Base::teardown();
	}
};

} // namespace testing

} // namespace stmi

#endif /* STMI_TESTING_FIXTURE_TEST_BASE_H */
