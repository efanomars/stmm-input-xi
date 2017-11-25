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
 * File:   xideviceswindow.h
 */

#ifndef STMI_XI_DEVICES_WINDOW_H
#define STMI_XI_DEVICES_WINDOW_H

#include "devicefloater.h"

#include <gtkmm.h>

#include <string>

namespace stmi
{

class XiDevicesWindow : public Gtk::Window
{
public:
	XiDevicesWindow(const Glib::ustring sTitle, int32_t nAcquireSeconds);
	virtual ~XiDevicesWindow();

	static constexpr int32_t s_nDefaultAcquireSeconds = 10;
	static constexpr int32_t s_nMaxAcquireSeconds = 5 * 60;
private:
	void recreateList();
	void devicesChanged();
	void slaveAdded(const DeviceFloater::SlaveDeviceData& oData);
	void onAcquireStartClicked();
	bool onAcquiringTimeout();
	void onAboutClicked();
private:
	Gtk::VBox m_oMainVBox;
	Gtk::HBox m_oAliveHBox;
		Gtk::Button m_oAcquireButton;
		Gtk::Label m_oCommentLabel;
		Gtk::ToggleButton m_oAboutButton;
	Gtk::ScrolledWindow m_oScrolled;
		Gtk::TreeView m_oTreeView;

	class DeviceColumns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		DeviceColumns()
		{
			add(m_oColIsFloating);
			add(m_oColTypeHidden);
			add(m_oColDeviceId);
			add(m_oColName);
			add(m_oColTotKeys);
		}
		Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > m_oColIsFloating;
		Gtk::TreeModelColumn<int32_t> m_oColTypeHidden;
		Gtk::TreeModelColumn<Glib::ustring> m_oColName;
		Gtk::TreeModelColumn<Glib::ustring> m_oColDeviceId; // DeviceId[MasterId], ex. 8[3]
		Gtk::TreeModelColumn<int32_t> m_oColTotKeys; // number of keys supported
	};
	DeviceColumns m_oDeviceColumns;
	Glib::RefPtr<Gtk::ListStore> m_refTreeModelDevices;

	DeviceFloater m_oFloater;

	Glib::ustring m_sIdleComment;
	Glib::ustring m_sAquiringComment;
	Glib::ustring m_sAboutComment;
	//
	Glib::ustring m_sAquireStartButton;
	Glib::ustring m_sAquiringButton;

	const int32_t m_nAcquireSeconds;
	bool m_bAcquiring;
	int32_t m_nAcquiringSecondsLeft;
	sigc::connection m_oSecondsTickConnection;

	static constexpr int32_t s_nInitialWindowSizeW = 600;
	static constexpr int32_t s_nInitialWindowSizeH = 600;
};

} // namespace stmi

#endif /* STMI_XI_DEVICES_WINDOW_H */

