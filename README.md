stmm-input-xi
=============

Multiple independent keyboards for the stmm-input framework and xinput (X11).

For more info visit http://www.efanomars.com/libraries/stmm-input-xi

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
Many Linux distributions offer the option to use the Wayland display
manager instead of X11. When run with Wayland xinput is just simulated
and floating a device does nothing. To avoid unnecessarily loading
libstmm-input-gtk-xi as a plugin you can disable it with the command
'stmm-input-plugins -d stmm-input-gtk-xi'.



Warning
-------
The APIs of the libraries aren't stable yet.
