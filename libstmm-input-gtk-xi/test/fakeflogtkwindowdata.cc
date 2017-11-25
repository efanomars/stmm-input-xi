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
 * File:   fakeflogtkwindowdata.cc
 */

#include "fakeflogtkwindowdata.h"

namespace stmi
{

namespace testing
{
namespace Flo
{

void FakeGtkWindowData::enable(const shared_ptr<GtkAccessor>& refAccessor, FloGtkDeviceManager* p0Owner)
{
	assert(refAccessor);
	assert(p0Owner != nullptr);
	setOwner(p0Owner);
	m_refAccessor = refAccessor;
	assert(m_p0Factory != nullptr);
	m_bIsRealized = m_p0Factory->m_bRealizedOnEnable;
	if (m_bIsRealized) {
		m_nXWinId = getNewWinId();
	}
	m_bIsEnabled= true;
}

} // namespace Flo
} // namespace Private

} // namespace stmi
