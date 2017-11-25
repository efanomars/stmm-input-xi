stmm-input-gtk-xi                                                  {#mainpage}
=================

Implementation of the stmm-input library for Gtkmm and X11 keyboards.

The device manager provided by this library sends key events defined in the
stmm-input-ev library to listeners.
It optionally can be used as a plugin (of stmm-input-dl).

While normally multiple keyboards plugged into the same computer are seen as
one keyboard, this library identifies the key events as coming from different
devices allowing multiple players to use the same keys to control games.
This provided that the keyboard is "floated" with the 'xinput float' command
or the device-floater GUI.

The device manager attaches itself to the Gtk event loop.


Important!
----------
Many Linux distributions are transitioning from X11 to Wayland.
While xinput is emulated in Wayland, it no longer recognizes additional
(usb) keyboards plugged into a computer as slaves of the master keyboard.
This prevents them from being floated and used independently.



Warning
-------
The API of this library isn't stable yet.
