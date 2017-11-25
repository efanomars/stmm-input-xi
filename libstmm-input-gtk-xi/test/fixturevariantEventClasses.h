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
 * File:   fixturevariantEventClasses.h
 */

#ifndef STMI_TESTING_FIXTURE_VARIANT_EVENT_CLASSES_H
#define STMI_TESTING_FIXTURE_VARIANT_EVENT_CLASSES_H

#include <stmm-input-ev/stmm-input-ev.h>

namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

namespace testing
{


class FixtureVariantEventClassesEnable_True
{
};
class FixtureVariantEventClassesEnable_False
{
};
class FixtureVariantEventClassesEnable
{
public:
	virtual ~FixtureVariantEventClassesEnable() = default;
protected:
	bool getEnable()
	{
		return (dynamic_cast<FixtureVariantEventClassesEnable_True*>(this) != nullptr);
	}
};

class FixtureVariantEventClasses_DeviceMgmtEvent
{
};
class FixtureVariantEventClasses_KeyEvent
{
};
class FixtureVariantEventClasses_PointerEvent
{
};
class FixtureVariantEventClasses_PointerScrollEvent
{
};
class FixtureVariantEventClasses_TouchEvent
{
};
class FixtureVariantEventClasses_JoystickButtonEvent
{
};
class FixtureVariantEventClasses_JoystickHatEvent
{
};
class FixtureVariantEventClasses_JoystickAxisEvent
{
};
class FixtureVariantEventClasses
{
public:
	virtual ~FixtureVariantEventClasses() = default;
protected:
	std::vector<Event::Class> getEventClasses()
	{
		std::vector<Event::Class> aClasses;
		if (dynamic_cast<FixtureVariantEventClasses_DeviceMgmtEvent*>(this) != nullptr) {
			aClasses.push_back(DeviceMgmtEvent::getClass());
		}
		if (dynamic_cast<FixtureVariantEventClasses_KeyEvent*>(this) != nullptr) {
			aClasses.push_back(KeyEvent::getClass());
		}
		if (dynamic_cast<FixtureVariantEventClasses_PointerEvent*>(this) != nullptr) {
			aClasses.push_back(PointerEvent::getClass());
		}
		if (dynamic_cast<FixtureVariantEventClasses_PointerScrollEvent*>(this) != nullptr) {
			aClasses.push_back(PointerScrollEvent::getClass());
		}
		if (dynamic_cast<FixtureVariantEventClasses_TouchEvent*>(this) != nullptr) {
			aClasses.push_back(TouchEvent::getClass());
		}
		if (dynamic_cast<FixtureVariantEventClasses_JoystickButtonEvent*>(this) != nullptr) {
			aClasses.push_back(JoystickButtonEvent::getClass());
		}
		if (dynamic_cast<FixtureVariantEventClasses_JoystickHatEvent*>(this) != nullptr) {
			aClasses.push_back(JoystickHatEvent::getClass());
		}
		if (dynamic_cast<FixtureVariantEventClasses_JoystickAxisEvent*>(this) != nullptr) {
			aClasses.push_back(JoystickAxisEvent::getClass());
		}
		return aClasses;
	}
};

} // namespace testing

} // namespace stmi

#endif /* STMI_TESTING_FIXTURE_VARIANT_EVENT_CLASSES_H */
