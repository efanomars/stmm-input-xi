/*
 * Copyright © 2016-2019  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   flogtkdevicemanager.cc
 */

#include "flogtkdevicemanager.h"

#include "flogtkbackend.h"
#include "flogtkwindowdata.h"
#include "flogtkxkeyboarddevice.h"

#include <stmm-input-ev/devicemgmtevent.h>
#include <stmm-input-ev/keycapability.h>
#include <stmm-input-ev/keyevent.h>
#include <stmm-input-gtk/gdkkeyconverter.h>
#include <stmm-input-gtk/gtkaccessor.h>
#include <stmm-input/capability.h>
#include <stmm-input/devicemanager.h>

#include <gdkmm/display.h>

#include <glibmm/ustring.h>

#include <cassert>
#include <algorithm>
#include <iterator>
#include <type_traits>

#include <stdlib.h>

namespace stmi { class Accessor; }
namespace stmi { class ChildDeviceManager; }

namespace stmi
{

using Private::Flo::GtkBackend;
using Private::Flo::GtkWindowData;
using Private::Flo::GtkWindowDataFactory;
using Private::Flo::GtkXKeyboardDevice;
using Private::Flo::XIEventSource;
using Private::Flo::FloGtkListenerExtraData;

std::string Private::Flo::checkIsNotWayland() noexcept
{
	const auto p0Value = ::getenv("WAYLAND_DISPLAY");
	if (p0Value != nullptr) {
		// variable defined
		if ((*p0Value) != 0) {
			// not empty string
			std::string sError =    "Error: Creation failed because on a Wayland system.\n"
									"       FloGtkDeviceManager only works properly on X11.";
			return sError; //---------------------------------------------------
		}
	}
	return "";
}
using Private::Flo::checkIsNotWayland;

std::pair<shared_ptr<FloGtkDeviceManager>, std::string> FloGtkDeviceManager::create(bool bEnableEventClasses, const std::vector<Event::Class>& aEnDisableEventClasses
																					, KeyRepeat::MODE eKeyRepeatMode, const shared_ptr<GdkKeyConverter>& refGdkConverter
																					, const Glib::RefPtr<Gdk::Display>& refDisplay) noexcept
{
	std::string sError = checkIsNotWayland();
	if (! sError.empty()) {
		return std::make_pair(shared_ptr<FloGtkDeviceManager>{}, sError); //----
	}
	shared_ptr<FloGtkDeviceManager> refInstance(new FloGtkDeviceManager(bEnableEventClasses, aEnDisableEventClasses
																		, eKeyRepeatMode, refGdkConverter));
	auto refBackend = std::make_unique<GtkBackend>(refInstance.get());
	sError = refBackend->init(refDisplay);
	if (! sError.empty()) {
		return std::make_pair(shared_ptr<FloGtkDeviceManager>{}, sError); //----
	}
	auto refFactory = std::make_unique<GtkWindowDataFactory>();
	refInstance->init(refFactory, refBackend);
	return std::make_pair(refInstance, "");
}
std::pair<shared_ptr<FloGtkDeviceManager>, std::string> FloGtkDeviceManager::create(const std::string& /*sAppName*/
																					, bool bEnableEventClasses
																					, const std::vector<Event::Class>& aEnDisableEventClasses) noexcept
{
	std::string sError = checkIsNotWayland();
	if (! sError.empty()) {
		return std::make_pair(shared_ptr<FloGtkDeviceManager>{}, sError); //----
	}
	shared_ptr<FloGtkDeviceManager> refInstance(new FloGtkDeviceManager(bEnableEventClasses, aEnDisableEventClasses
																		, KeyRepeat::getMode(), GdkKeyConverter::getConverter()));

	auto refBackend = std::make_unique<GtkBackend>(refInstance.get());
	sError = refBackend->init(Gdk::Display::get_default());
	if (! sError.empty()) {
		return std::make_pair(shared_ptr<FloGtkDeviceManager>{}, sError); //----
	}

	auto refFactory = std::make_unique<GtkWindowDataFactory>();
	refInstance->init(refFactory, refBackend);
	return std::make_pair(refInstance, "");
}
FloGtkDeviceManager::FloGtkDeviceManager(bool bEnableEventClasses, const std::vector<Event::Class>& aEnDisableEventClasses
										, KeyRepeat::MODE eKeyRepeatMode, const shared_ptr<GdkKeyConverter>& refGdkConverter) noexcept
: StdDeviceManager({Capability::Class{typeid(KeyCapability)}}
					, {Event::Class{typeid(DeviceMgmtEvent)}, Event::Class{typeid(KeyEvent)}}
					, bEnableEventClasses, aEnDisableEventClasses)
, m_nCancelingNestedDepth(0)
, m_eKeyRepeatMode((eKeyRepeatMode == KeyRepeat::MODE_NOT_SET) ? KeyRepeat::getMode() : eKeyRepeatMode)
, m_refGdkConverter(refGdkConverter)
, m_oConverter(*m_refGdkConverter)
, m_nClassIdxKeyEvent(getEventClassIndex(Event::Class{typeid(KeyEvent)}))
{
	assert(refGdkConverter);
	assert((m_eKeyRepeatMode >= KeyRepeat::MODE_SUPPRESS) && (m_eKeyRepeatMode <= KeyRepeat::MODE_ADD_RELEASE_CANCEL));
	// The whole implementation of this class is based on this assumption
	static_assert(sizeof(int) <= sizeof(int32_t), "");
}
void FloGtkDeviceManager::enableEventClass(const Event::Class& oEventClass) noexcept
{
	StdDeviceManager::enableEventClass(oEventClass);
	adjustConnectionsAfterEnablingClass();
}
void FloGtkDeviceManager::adjustConnectionsAfterEnablingClass() noexcept
{
//std::cout << "FloGtkDeviceManager::adjustConnectionsAfterEnablingClass(" << (int64_t)this << ")" << std::endl;
	if (!isEventClassEnabled(Event::Class{typeid(KeyEvent)})) {
		return;
	}
	for (auto& oPair : m_aGtkWindowData) { //m_aXWindowData
		auto& refWindowData = oPair.second;
		if (! refWindowData->isEnabled()) {
			continue; // for --------
		}
		auto& refAccessor = refWindowData->getAccessor();
		if (refAccessor->isDeleted()) {
			continue; // for --------
		}
		refWindowData->connectAllDevices();
	}
	if (m_refSelected) {
		selectAccessor(m_refSelected);
	}
}
void FloGtkDeviceManager::init(std::unique_ptr<Private::Flo::GtkWindowDataFactory>& refFactory
								, std::unique_ptr<Private::Flo::GtkBackend>& refBackend) noexcept
{
	assert(refFactory);
	assert(refBackend);
	m_refFactory.swap(refFactory);
	m_refBackend.swap(refBackend);
	addDevices();
}
void FloGtkDeviceManager::addDevices() noexcept
{
	auto& aXdDeviceIds = m_refBackend->getXDeviceIds();
	assert(m_aKeyboardDevices.empty());
	for (auto nXDeviceId : aXdDeviceIds) {
		std::string sDeviceName = m_refBackend->getDeviceName(nXDeviceId);
		addGtkXKeyboardDevice(nXDeviceId, sDeviceName);
	}
}
shared_ptr<GtkXKeyboardDevice> FloGtkDeviceManager::addGtkXKeyboardDevice(int nXDeviceId, const Glib::ustring& sDeviceName) noexcept
{
	#ifndef NDEBUG
	std::vector< std::pair<int32_t, shared_ptr<Private::Flo::GtkXKeyboardDevice> > >::iterator itFind;
	const bool bDeviceFound = findDevice(nXDeviceId, itFind);
	assert(!bDeviceFound);
	#endif

	shared_ptr<ChildDeviceManager> refChildThis = shared_from_this();
	assert(dynamic_cast<FloGtkDeviceManager*>(refChildThis.get()));
	auto refThis = std::static_pointer_cast<FloGtkDeviceManager>(refChildThis);
	//
	auto refGtkXKeyboard = std::make_shared<GtkXKeyboardDevice>(sDeviceName, refThis);
	m_aKeyboardDevices.emplace_back(nXDeviceId, refGtkXKeyboard);
	//
	#ifndef NDEBUG
	const bool bAdded = 
	#endif
	StdDeviceManager::addDevice(refGtkXKeyboard);
	assert(bAdded);
	// for each accessor (window) select xi events
	connectDeviceToAllWindows(nXDeviceId);
	return refGtkXKeyboard;
}
void FloGtkDeviceManager::removeGtkXKeyboardDevice(int32_t nIdx) noexcept
{
	const int32_t nLastIdx = static_cast<int32_t>(m_aKeyboardDevices.size()) - 1;
	if (nIdx < nLastIdx) {
		m_aKeyboardDevices[nIdx].swap(m_aKeyboardDevices[nLastIdx]);
	}
	m_aKeyboardDevices.pop_back();
}
void FloGtkDeviceManager::onDeviceChanged(int32_t nXDeviceId) noexcept
{
	std::vector< std::pair<int32_t, shared_ptr<Private::Flo::GtkXKeyboardDevice> > >::iterator itFind;
	#ifndef NDEBUG
	const bool bFound =
	#endif
	findDevice(nXDeviceId, itFind);
	assert(bFound);
	auto& refGtkXKeyboard = itFind->second;
	sendDeviceMgmtToListeners(DeviceMgmtEvent::DEVICE_MGMT_CHANGED, refGtkXKeyboard);

}
void FloGtkDeviceManager::onDeviceAdded(int32_t nXDeviceId) noexcept
{
	auto refGtkXKeyboard = addGtkXKeyboardDevice(nXDeviceId, m_refBackend->getDeviceName(nXDeviceId));
	sendDeviceMgmtToListeners(DeviceMgmtEvent::DEVICE_MGMT_ADDED, refGtkXKeyboard);
}
void FloGtkDeviceManager::onDeviceRemoved(int32_t nXDeviceId) noexcept
{
	std::vector< std::pair<int32_t, shared_ptr<Private::Flo::GtkXKeyboardDevice> > >::iterator itFind;
	#ifndef NDEBUG
	const bool bFound =
	#endif
	findDevice(nXDeviceId, itFind);
	assert(bFound);
	auto& refGtkXKeyboard = itFind->second;

	cancelDeviceKeys(refGtkXKeyboard);
	refGtkXKeyboard->removingDevice();

	auto refRemovedGtkXKeyboard = refGtkXKeyboard;
	#ifndef NDEBUG
	const bool bRemoved =
	#endif //NDEBUG
	StdDeviceManager::removeDevice(refGtkXKeyboard);
	assert(bRemoved);
	//
	const int32_t nIdx = std::distance(m_aKeyboardDevices.begin(), itFind);
	removeGtkXKeyboardDevice(nIdx);
	//
	sendDeviceMgmtToListeners(DeviceMgmtEvent::DEVICE_MGMT_REMOVED, refRemovedGtkXKeyboard);
}
bool FloGtkDeviceManager::onXIDeviceEvent(XIDeviceEvent* p0XIDeviceEvent) noexcept
{
//std::cout << "FloGtkDeviceManager::onXIDeviceEvent" << '\n';
	const bool bContinue = true;
	const int32_t nXDeviceId = p0XIDeviceEvent->deviceid;
	std::vector< std::pair<int32_t, shared_ptr<Private::Flo::GtkXKeyboardDevice> > >::iterator itFind;
	#ifndef NDEBUG
	const bool bFound =
	#endif
	findDevice(nXDeviceId, itFind);
	assert(bFound);
	auto& refGtkXKeyboard = itFind->second;
	if ((!m_refSelected) || (m_refSelected->getXWindow() != p0XIDeviceEvent->event)) {
		// shouldn't happen
		return bContinue; //----------------------------------------------------
	}
	return refGtkXKeyboard->handleXIDeviceEvent(p0XIDeviceEvent, m_refSelected);
}
void FloGtkDeviceManager::finalizeListener(ListenerData& oListenerData) noexcept
{
	++m_nCancelingNestedDepth;
	const int64_t nEventTimeUsec = DeviceManager::getNowTimeMicroseconds();
	for (auto& oPair : m_aKeyboardDevices) {
		shared_ptr<GtkXKeyboardDevice>& refGtkXKeyboard = oPair.second;
		refGtkXKeyboard->finalizeListener(oListenerData, nEventTimeUsec);
	}
	--m_nCancelingNestedDepth;
	if (m_nCancelingNestedDepth == 0) {
		resetExtraDataOfAllListeners();
	}
}
bool FloGtkDeviceManager::findDevice(int32_t nXDeviceId
				, std::vector< std::pair<int32_t, shared_ptr<GtkXKeyboardDevice> > >::iterator& itFind) noexcept
{
	itFind = std::find_if(m_aKeyboardDevices.begin(), m_aKeyboardDevices.end(),
			[&](const std::pair<int32_t, shared_ptr<GtkXKeyboardDevice> >& oPair)
			{
				return (oPair.first == nXDeviceId);
			});
	return (itFind != m_aKeyboardDevices.end());
}
bool FloGtkDeviceManager::findWindow(Gtk::Window* p0GtkmmWindow
				, std::vector< std::pair<Gtk::Window*, shared_ptr<GtkWindowData> > >::iterator& itFind) noexcept
{
	itFind = std::find_if(m_aGtkWindowData.begin(), m_aGtkWindowData.end(),
			[&](const std::pair<Gtk::Window*, shared_ptr<GtkWindowData> >& oPair)
			{
				return (oPair.first == p0GtkmmWindow);
			});
	return (itFind != m_aGtkWindowData.end());
}
bool FloGtkDeviceManager::hasAccessor(const shared_ptr<Accessor>& refAccessor, bool& bValid
				, std::vector< std::pair<Gtk::Window*, shared_ptr<GtkWindowData> > >::iterator& itFind) noexcept
{
	bValid = false;
	if (!refAccessor) {
		return false; //--------------------------------------------------------
	}
	shared_ptr<GtkAccessor> refGtkAccessor = std::dynamic_pointer_cast<GtkAccessor>(refAccessor);
	if (!refGtkAccessor) {
		return false; //--------------------------------------------------------
	}
	bValid = true;
	Gtk::Window* p0GtkmmWindow = refGtkAccessor->getGtkmmWindow();
	const bool bFoundWindow = findWindow(p0GtkmmWindow, itFind);
	return bFoundWindow;
}
bool FloGtkDeviceManager::hasAccessor(const shared_ptr<Accessor>& refAccessor) noexcept
{
	bool bValid;
	std::vector<std::pair<Gtk::Window*, shared_ptr<GtkWindowData> > >::iterator itFind;
	return hasAccessor(refAccessor, bValid, itFind);
}
bool FloGtkDeviceManager::addAccessor(const shared_ptr<Accessor>& refAccessor) noexcept
{
//std::cout << "FloGtkDeviceManager::addAccessor(" << (int64_t)this << ")" << std::endl;
	bool bValid;
	std::vector<std::pair<Gtk::Window*, shared_ptr<GtkWindowData> > >::iterator itFind;
	const bool bHasAccessor = hasAccessor(refAccessor, bValid, itFind);
	if (!bValid) {
		return false; //--------------------------------------------------------
	}
	shared_ptr<GtkAccessor> refGtkAccessor = std::static_pointer_cast<GtkAccessor>(refAccessor);
	if (refGtkAccessor->isDeleted()) {
		// the window was already destroyed, unusable
		// but if a zombie GtkAccessorData is still around, remove it
		if (bHasAccessor) {
			removeAccessor(itFind);
		}
		return false; //--------------------------------------------------------
	}
	if (bHasAccessor) {
		// Accessor is already present
		return false; //--------------------------------------------------------
	}
	if (!m_refBackend->isCompatible(refGtkAccessor)) {
		// wrong display
		return false; //--------------------------------------------------------
	}

	Gtk::Window* p0GtkmmWindow = refGtkAccessor->getGtkmmWindow();
	m_aGtkWindowData.emplace_back(p0GtkmmWindow, m_refFactory->create());
	shared_ptr<GtkWindowData> refData = m_aGtkWindowData.back().second;
	GtkWindowData& oData = *refData;

	oData.enable(refGtkAccessor, this);

	const bool bIsActive = oData.isWindowActive();
	if (!m_refSelected) {
		if (bIsActive) {
			if (isEventClassEnabled(Event::Class{typeid(KeyEvent)})) {
				refData->connectAllDevices();
			}
			selectAccessor(refData);
		}
	} else {
		if (bIsActive) {
			auto refSelectedAccessor = m_refSelected->getAccessor();
			if (refSelectedAccessor->isDeleted()) {
				const auto p0SelectedGtkmmWindow = refSelectedAccessor->getGtkmmWindow();
				std::vector< std::pair<Gtk::Window*, shared_ptr<GtkWindowData> > >::iterator itSelectedFind;
				#ifndef NDEBUG
				const bool bFoundWindow = 
				#endif //NDEBUG
				findWindow(p0SelectedGtkmmWindow, itSelectedFind);
				assert(bFoundWindow);
				removeAccessor(itSelectedFind);
			} else {
				// This shouldn't happen: the added window is active while another is still selected.
				deselectAccessor();
			}
			if (isEventClassEnabled(Event::Class{typeid(KeyEvent)})) {
				refData->connectAllDevices();
			}
			selectAccessor(refData);
		} else {
			deselectAccessor();
		}
	}
//std::cout << "FloGtkDeviceManager::addAccessor()   m_aGtkWindowData.size()=" << m_aGtkWindowData.size() << std::endl;
	return true;
}
bool FloGtkDeviceManager::removeAccessor(const shared_ptr<Accessor>& refAccessor) noexcept
{
	std::vector<std::pair<Gtk::Window*, shared_ptr<GtkWindowData> > >::iterator itFind;
	bool bValid;
	const bool bHasAccessor = hasAccessor(refAccessor, bValid, itFind);
	if (!bHasAccessor) {
		return false; //--------------------------------------------------------
	}
	removeAccessor(itFind);
	return true;
}
void FloGtkDeviceManager::removeAccessor(const std::vector<std::pair<Gtk::Window*, shared_ptr<GtkWindowData> > >::iterator& itGtkData) noexcept
{
//std::cout << "FloGtkDeviceManager::removeAccessor()   m_aGtkWindowData.size()=" << m_aGtkWindowData.size() << std::endl;
	// Note: an additional shared_ptr to the object is created to avoid it
	//       being recycled during deselectAccessor()
	shared_ptr<GtkWindowData> refData = itGtkData->second;

	const bool bIsSelected = (m_refSelected == refData);

	refData->disable(); // doesn't clear accessor!

	//
	m_aGtkWindowData.erase(itGtkData);

	if (bIsSelected) {
		// refData reference avoids recycling during cancel callbacks!
		deselectAccessor();
	}
}
void FloGtkDeviceManager::selectAccessor(const shared_ptr<GtkWindowData>& refData) noexcept
{
//std::cout << "FloGtkDeviceManager::selectAccessor  accessor=" << (int64_t)&(*(refData->getAccessor())) << "  XWinId=" << refData->getXWindow() << std::endl;
	m_refSelected = refData;
	focusSelectedWindow();
}
void FloGtkDeviceManager::deselectAccessor() noexcept
{
//std::cout << "FloGtkDeviceManager::deselectAccessor  m_nCancelingNestedDepth=" << m_nCancelingNestedDepth << '\n';
	++m_nCancelingNestedDepth;
	//
	for (auto& oPair : m_aKeyboardDevices) {
		shared_ptr<GtkXKeyboardDevice>& refKeyboard = oPair.second;
		// cancel all keys that are pressed for the currently selected accessor
		refKeyboard->cancelSelectedAccessorKeys();
	}
	m_refSelected.reset();
	focusSelectedWindow();
	//
	--m_nCancelingNestedDepth;
	if (m_nCancelingNestedDepth == 0) {
		resetExtraDataOfAllListeners();
	}
}
void FloGtkDeviceManager::cancelDeviceKeys(const shared_ptr<GtkXKeyboardDevice>& refGtkXKeyboard) noexcept
{
//std::cout << "FloGtkDeviceManager::cancelDeviceKeys  m_nCancelingNestedDepth=" << m_nCancelingNestedDepth << std::endl;
	++m_nCancelingNestedDepth;
	//
	refGtkXKeyboard->cancelSelectedAccessorKeys();
	//
	--m_nCancelingNestedDepth;
	if (m_nCancelingNestedDepth == 0) {
		resetExtraDataOfAllListeners();
	}
}
void FloGtkDeviceManager::onIsActiveChanged(const shared_ptr<GtkWindowData>& refWindowData) noexcept
{
//std::cout << "FloGtkDeviceManager::onIsActiveChanged  accessor=" << (int64_t)&(*(refWindowData->getAccessor()));
	if (!refWindowData->isEnabled()) {
//std::cout << '\n';
		return;
	}
	bool bIsActive = refWindowData->isWindowActive();
//std::cout << "   active=" << bIsActive << '\n';
	const bool bIsSelected = (refWindowData == m_refSelected);
	if (bIsActive) {
		if (bIsSelected) {
			// Activating the already active window ... shouldn't happen.
			return; //----------------------------------------------------------
		}
		if (m_refSelected) {
			deselectAccessor();
		}
		if (isEventClassEnabled(Event::Class{typeid(KeyEvent)})) {
			refWindowData->connectAllDevices();
		}
		selectAccessor(refWindowData);
	} else {
		if (bIsSelected) {
			// Send cancels for open keys, buttons and sequences
			deselectAccessor();
		}
	}
}
void FloGtkDeviceManager::focusSelectedWindow() noexcept
{
	if (isEventClassEnabled(Event::Class{typeid(KeyEvent)})) {
		m_refBackend->focusDevicesToWindow(m_refSelected);
	}
}
void FloGtkDeviceManager::connectDeviceToAllWindows(int32_t nXDeviceId) noexcept
{
//std::cout << "FloGtkDeviceManager::connectDeviceToAllWindows()  nXDeviceId=" << nXDeviceId << std::endl;
	//
	for (auto& oPair : m_aGtkWindowData) { //m_aXWindowData
		auto& refWindowData = oPair.second;
		if (! refWindowData->isEnabled()) {
			continue; // for --------
		}
		refWindowData->connectDevice(nXDeviceId);
	}
}

} // namespace stmi
