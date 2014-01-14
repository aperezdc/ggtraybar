GGT - GNOME-inspired Geeky Traybar
==================================

This is a tiny replacement for the GNOME Panel. It was designed to be
simple, light and work nicely with the OpenBox window manager. Currently
it has the following features:

- Top aligned, full-width panel.

- Command launcher bound globally to "Alt-F2", like the GNOME run dialog.

- Task list.

- Pager with desktop previews like the one in the GNOME Panel.

- System tray for application icons.

- 24-hour clock with calendar pop-up (date can be found in the tooltip).

- Multi-monitor (XRandR) support. The panel will occupy only the primary
	monitor, and when the screen layout or the number of monitors are changed,
	it will adapt itself to the new setup.

- No configuration.

To use it with OpenBox, just launch "ggt" from the "autostart.sh" script.
You may want to launch other additional goodies from there as well like
nm-applet, gnome-settings-daemon, gnome-screensaver, gnome-power-manager, etc.


Dependencies
------------

You will need the following to build GGTraybar:

- A recent enough GTK+ (3.8 or newer) -- http://gtk.org
- libwnck, tested with 3.4 -- this is part of GNOME.
- keybinder, tested with 0.3.0 -- http://kaizer.se/wiki/keybinder/


Copyright
---------

GGTraybar is (C) 2010-2014 Adrian Perez <aperez@igalia.com>, see COPYING.

Portions from other authors:

- ``eggtraymanager.[hc]``: Copyright (C) 2002 Anders Carlsson <andersca@gnu.org>


