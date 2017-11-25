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
 * File:   floatingsources.h
 */

#ifndef STMI_FLOATING_SOURCES_H
#define STMI_FLOATING_SOURCES_H

#include <gtkmm.h>

#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>

#include <memory>

namespace stmi
{

namespace Private
{
namespace Flo
{

using std::shared_ptr;
using std::weak_ptr;

/* XInput2 Source for floating device events */
class XIEventSource : public Glib::Source
{
public:
	XIEventSource(Gdk::Display* p0Display, int nXiOpcode);
	virtual ~XIEventSource();

	// A source can have only one callback type, that is the slot given as parameter
	sigc::connection connect(const sigc::slot<bool, XIDeviceEvent*>& oSlot);
protected:
	bool prepare(int& nTimeout) override;
	bool check() override;
	bool dispatch(sigc::slot_base* slot) override;

private:
	bool checkEvent(XEvent& oXEvent, bool& bIsGenericEvent, bool& bIsForFloating, bool& bIsFloatingEnterFocus);

private:
	::Display* m_p0XDisplay;
	Glib::PollFD m_oPollFd;
	const int m_nXiOpcode;
	//
	bool m_bSkipNextEvent;

private:
	XIEventSource() = delete;
	XIEventSource(const XIEventSource& oSource) = delete;
	XIEventSource& operator=(const XIEventSource& oSource) = delete;
};

} // namespace Flo
} // namespace Private

} // namespace stmi

#endif /* STMI_FLOATING_SOURCES_H */
