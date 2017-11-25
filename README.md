stmm-input-xi
=============

Multiple independent keyboards for the stmm-input framework and xinput (X11).


This source package contains:

- libstmm-input-gtk-xi:
    library that implements a device manager that integrates into Gtk's
    main event loop sending events defined by libstmm-input-ev to listeners.
    The device manager handles multiple independent keyboard devices
    readied with device-floater.
    The library can be stand-alone or loaded as a plugin by libstmm-input-dl
    (stmm-input repository).

- device-floater:
    simple gui application to safely float keyboard and pointer devices
    managed by X11 (XI2). It provides a subset of the 'xinput float' command.


Read the INSTALL file for installation instructions.

An example can be found in the libstmm-input-gtk-xi subfolder.


Important note!
---------------
Many Linux distributions are transitioning from X11 to Wayland.
While xinput is emulated in Wayland, it no longer recognizes additional
(usb) keyboards plugged into a computer as slaves of the master keyboard.
This prevents them from being floated and used independently.
To find out whether your distribution is based on X11 or Wayland type
the command 'xinput' in your terminal: if the device names contain the
word 'Wayland' then you shouldn't install the contents of this package.



Warning
-------
The APIs of the libraries aren't stable yet.
