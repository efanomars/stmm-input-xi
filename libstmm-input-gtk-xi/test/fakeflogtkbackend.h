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
 * File:   fakeflogtkbackend.h
 */

#ifndef STMI_TESTING_FAKE_FLO_GTK_BACKEND_H
#define STMI_TESTING_FAKE_FLO_GTK_BACKEND_H

#include "flogtkbackend.h"

namespace stmi
{

using std::shared_ptr;
using std::weak_ptr;

namespace testing
{
namespace Flo
{

class FakeGtkBackend : public Private::Flo::GtkBackend
{
public:
	FakeGtkBackend(::stmi::FloGtkDeviceManager* p0Owner, const Glib::RefPtr<Gdk::Display>& refGdkDisplay);
	std::string getDeviceName(int32_t nXDeviceId) const override
	{
		#ifndef NDEBUG
		const int32_t nIdx = 
		#endif
		findXDeviceId(nXDeviceId);
		assert(nIdx >= 0);
		std::ostringstream o;
		if (!(o << nXDeviceId)) {
			assert(false);
		}
		return "TestPointer~" + o.str();
	}
	bool isCompatible(const shared_ptr<GtkAccessor>& refGtkAccessor) const override
	{
		Gtk::Window* p0GtkmmWindow = refGtkAccessor->getGtkmmWindow();
		assert(p0GtkmmWindow != nullptr);
		Glib::RefPtr<Gdk::Display> refDisplay = p0GtkmmWindow->get_display();
		assert(refDisplay);
		return (m_refGdkDisplay == refDisplay);
	}
	const std::vector< int32_t >& getXDeviceIds() override
	{
		return m_aXDeviceIds;
	}
	void focusDevicesToWindow(const std::shared_ptr<Private::Flo::GtkWindowData>& refWinData) override;
	//
	int32_t simulateNewDevice()
	{
		static int32_t s_nXDeviceId = 0;
		//
		const int32_t nXDeviceId = ++s_nXDeviceId;
		//
		m_aXDeviceIds.push_back(nXDeviceId);
		onDeviceAdded(nXDeviceId);
		return nXDeviceId;
	}
	void simulateRemoveDevice(int32_t nXDeviceId)
	{
		const int32_t nIdx = findXDeviceId(nXDeviceId);
		assert(nIdx >= 0);
		const int32_t nLastIdx = static_cast<int32_t>(m_aXDeviceIds.size()) - 1;
		if (nIdx < nLastIdx) {
			m_aXDeviceIds[nIdx] = m_aXDeviceIds[nLastIdx];
		}
		m_aXDeviceIds.pop_back();
		onDeviceRemoved(nXDeviceId);
	}
	void simulateDeviceChanged(int32_t nXDeviceId)
	{
		#ifndef NDEBUG
		const int32_t nIdx = 
		#endif
		findXDeviceId(nXDeviceId);
		assert(nIdx >= 0);
		onDeviceChanged(nXDeviceId);
	}
	void simulateKeyEvent(int32_t nXDeviceId, ::Window nWinId, bool bPress, int32_t nHardwareCode);
private:
	int32_t findXDeviceId(int32_t nXDeviceId) const
	{
		auto itFind = std::find(m_aXDeviceIds.begin(), m_aXDeviceIds.end(), nXDeviceId);
		if (itFind == m_aXDeviceIds.end()) {
			return -1;
		}
		return std::distance(m_aXDeviceIds.begin(), itFind);
	}
private:
	const Glib::RefPtr<Gdk::Display> m_refGdkDisplay;

	std::vector< int32_t > m_aXDeviceIds;
	::Window m_nFocusXWinId;
};

} // namespace Flo
} // namespace testing

} // namespace stmi

#endif /* STMI_TESTING_FAKE_FLO_GTK_BACKEND_H */
