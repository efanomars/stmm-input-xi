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
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   fixturevariantKeyRepeatMode.h
 */

#ifndef STMI_TESTING_FIXTURE_VARIANT_KEY_REPEAT_MODE_H
#define STMI_TESTING_FIXTURE_VARIANT_KEY_REPEAT_MODE_H

#include <stmm-input-gtk/keyrepeatmode.h>

namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

namespace testing
{

class FixtureVariantKeyRepeatMode_Suppress
{
};
class FixtureVariantKeyRepeatMode_AddRelease
{
};
class FixtureVariantKeyRepeatMode_AddReleaseCancel
{
};
class FixtureVariantKeyRepeatMode
{
public:
	virtual ~FixtureVariantKeyRepeatMode() = default;
protected:
	KeyRepeat::MODE getKeyRepeatMode()
	{
		KeyRepeat::MODE eKeyRepeatMode = KeyRepeat::MODE_SUPPRESS;
		if (dynamic_cast<FixtureVariantKeyRepeatMode_Suppress*>(this) != nullptr) {
			eKeyRepeatMode = KeyRepeat::MODE_SUPPRESS;
		} else if (dynamic_cast<FixtureVariantKeyRepeatMode_AddRelease*>(this) != nullptr) {
			eKeyRepeatMode = KeyRepeat::MODE_ADD_RELEASE;
		} else if (dynamic_cast<FixtureVariantKeyRepeatMode_AddReleaseCancel*>(this) != nullptr) {
			eKeyRepeatMode = KeyRepeat::MODE_ADD_RELEASE_CANCEL;
		}
		return eKeyRepeatMode;
	}
};

} // namespace testing

} // namespace stmi

#endif /* STMI_TESTING_FIXTURE_VARIANT_KEY_REPEAT_MODE_H */
