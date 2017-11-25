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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   xideviceswindow.cc
 */

#include "xideviceswindow.h"

#include "config.h"

#include <cassert>
#include <iostream>

namespace stmi
{

XiDevicesWindow::XiDevicesWindow(const Glib::ustring sTitle, int32_t nAcquireSeconds)
: m_oFloater(get_display())
, m_nAcquireSeconds(nAcquireSeconds)
, m_bAcquiring(false)
, m_nAcquiringSecondsLeft(0)
{
	assert(nAcquireSeconds >= 1);
	//
	set_title(sTitle);
	set_default_size(s_nInitialWindowSizeW, s_nInitialWindowSizeH);
	set_resizable(true);

	m_sAquireStartButton = "Acquire\n  Start";
	m_sAquiringButton = "Acquire\n  %1 sec";
	m_oAcquireButton.set_label(m_sAquireStartButton);

	m_sIdleComment = Glib::ustring::compose("After you press button 'Acquire Start' you have %1 sec to plug in\n", m_nAcquireSeconds);
	m_sIdleComment += "the keyboard and pointer devices you want to float.";
	m_sAquiringComment = "Floating devices are shown below on top of the list\n";
	m_sAquiringComment += "with the buoy icon. They are temporary, if you plug them out\n";
	m_sAquiringComment += "and in again while not acquiring they go back to normal.";
	m_sAboutComment = "Copyright © 2016 Stefano Marsili, <stemars@gmx.ch>\n";
	m_sAboutComment += "    GPL 3.0\n";
	m_sAboutComment += "\n";
	m_sAboutComment += "Icons made by Freepik (http://www.freepik.com).\n";
	m_sAboutComment += "    Creative Commons BY 3.0";
	m_oCommentLabel.set_text(m_sIdleComment);
	m_oCommentLabel.set_margin_left(5);
	m_oCommentLabel.set_margin_right(5);

	m_oAboutButton.set_label("@");

	m_oAliveHBox.set_margin_left(5);
	m_oAliveHBox.set_margin_right(5);
	m_oAliveHBox.set_margin_top(5);
	m_oAliveHBox.set_margin_bottom(2);
	m_oAliveHBox.pack_start(m_oAcquireButton, Gtk::PACK_SHRINK);
	m_oAliveHBox.pack_start(m_oCommentLabel);
	m_oAliveHBox.pack_start(m_oAboutButton, Gtk::PACK_SHRINK);

	m_refTreeModelDevices = Gtk::ListStore::create(m_oDeviceColumns);
	m_oTreeView.set_model(m_refTreeModelDevices);

	m_oTreeView.set_headers_visible(true);
	m_oTreeView.append_column("Type", m_oDeviceColumns.m_oColIsFloating);
	const int32_t nNameCol = m_oTreeView.append_column("Name", m_oDeviceColumns.m_oColName) - 1;
	m_oTreeView.append_column("Id [Master]", m_oDeviceColumns.m_oColDeviceId);
	m_oTreeView.append_column("Tot keys", m_oDeviceColumns.m_oColTotKeys);
	const int32_t nTotColumns = m_oTreeView.get_n_columns();
	for (int32_t nCol = 0; nCol < nTotColumns; ++nCol) {
		Gtk::TreeViewColumn* p0Column =  m_oTreeView.get_column(nCol);
		p0Column->set_reorderable(false);
		p0Column->set_clickable(false);
		p0Column->set_expand(nCol == nNameCol);
	}

	m_oScrolled.set_border_width(5);
	m_oScrolled.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
	m_oScrolled.add(m_oTreeView);

	m_oMainVBox.pack_start(m_oAliveHBox, Gtk::PACK_SHRINK);
	m_oMainVBox.pack_start(m_oScrolled);

	Gtk::Window::add(m_oMainVBox);
	show_all_children();

	m_oFloater.m_oDeviceChangedSignal.connect( sigc::mem_fun(this, &XiDevicesWindow::devicesChanged) );
	recreateList();
	m_oFloater.m_oSlaveAddedSignal.connect( sigc::mem_fun(this, &XiDevicesWindow::slaveAdded) );
	m_oAcquireButton.signal_clicked().connect( sigc::mem_fun(*this, &XiDevicesWindow::onAcquireStartClicked) );
	m_oAboutButton.signal_clicked().connect( sigc::mem_fun(*this, &XiDevicesWindow::onAboutClicked) );
}
XiDevicesWindow::~XiDevicesWindow()
{
}
void XiDevicesWindow::recreateList()
{
	m_refTreeModelDevices->clear();

	const std::vector<DeviceFloater::SlaveDeviceData> aDevices = m_oFloater.getDevices();
//std::cout << "XiDevicesWindow::recreateList()  aDevices.size()=" << aDevices.size() << std::endl;
	int32_t nIdx = 0;
	for (auto& oData : aDevices) {
		Gtk::ListStore::iterator itListDevice = m_refTreeModelDevices->append();
		std::string sIsFloatingFile = Config::getDataDir() + "/";
		switch (oData.m_eType) {
			case DeviceFloater::DEVICE_TYPE_FLOATING: { sIsFloatingFile += "buoy_32313.png"; } break;
			case DeviceFloater::DEVICE_TYPE_KEYBOARD: { sIsFloatingFile += "keyboard_1420.png"; } break;
			case DeviceFloater::DEVICE_TYPE_POINTER: { sIsFloatingFile += "mouse_347.png"; } break;
			default: assert(false);
		}
		if (!sIsFloatingFile.empty()) {
			(*itListDevice)[m_oDeviceColumns.m_oColIsFloating] = Gdk::Pixbuf::create_from_file(sIsFloatingFile);
		}
		(*itListDevice)[m_oDeviceColumns.m_oColTypeHidden] = oData.m_eType;
		(*itListDevice)[m_oDeviceColumns.m_oColName] = oData.m_sName;
		auto sDeviceId = Glib::ustring::compose("    %1", oData.m_nId);
		if (oData.m_eType == DeviceFloater::DEVICE_TYPE_FLOATING) {
			sDeviceId += " [F]";
			m_oTreeView.get_selection()->select(itListDevice);
		} else {
			sDeviceId += Glib::ustring::compose(" [%1]", oData.m_nMasterId);
			m_oTreeView.get_selection()->unselect(itListDevice);
		}
		sDeviceId += "    ";
		(*itListDevice)[m_oDeviceColumns.m_oColDeviceId] = sDeviceId;
		(*itListDevice)[m_oDeviceColumns.m_oColTotKeys] = oData.m_nTotKeys;
		++nIdx;
	}
}
void XiDevicesWindow::devicesChanged()
{
//std::cout << "XiDevicesWindow::devicedChanged()" << std::endl;
	recreateList();
}
void XiDevicesWindow::slaveAdded(const DeviceFloater::SlaveDeviceData& oData)
{
//std::cout << "XiDevicesWindow::slaveAdded() nId=" << oData.m_nId << std::endl;
	if (!m_bAcquiring) {
		return;
	}
	const bool bOk = m_oFloater.floatDevice(oData.m_nId);
	if (!bOk) {
		std::cout << "XI error: couldn't float device with Id=" << oData.m_nId << std::endl;
	}
}
void XiDevicesWindow::onAcquireStartClicked()
{
	if (m_bAcquiring) {
		m_oSecondsTickConnection.disconnect();
	} else {
		m_bAcquiring = true;
	}
	m_nAcquiringSecondsLeft = m_nAcquireSeconds;
	const bool bActive = m_oAboutButton.get_active();
	if (bActive) {
		m_oAboutButton.set_active(false);
	}
	onAboutClicked();
	m_oAcquireButton.set_label(Glib::ustring::compose(m_sAquiringButton, m_nAcquiringSecondsLeft));
	m_oSecondsTickConnection = Glib::signal_timeout().connect(
								sigc::mem_fun(*this, &XiDevicesWindow::onAcquiringTimeout), 1000);
}
bool XiDevicesWindow::onAcquiringTimeout()
{
	--m_nAcquiringSecondsLeft;
	if (m_nAcquiringSecondsLeft <= 0) {
		m_bAcquiring = false;
		m_oAcquireButton.set_label(m_sAquireStartButton);
	} else {
		m_oAcquireButton.set_label(Glib::ustring::compose(m_sAquiringButton, m_nAcquiringSecondsLeft));
	}
	onAboutClicked();
	return m_bAcquiring;
}
void XiDevicesWindow::onAboutClicked()
{
	const bool bActive = m_oAboutButton.get_active();
	if (bActive) {
		m_oCommentLabel.set_text(m_sAboutComment);
	} else {
		if (m_bAcquiring) {
			m_oCommentLabel.set_text(m_sAquiringComment);
		} else {
			m_oCommentLabel.set_label(m_sIdleComment);
		}
	}
}

} // namespace stmi

