# Based off original GNOME 2 applet build files copyright:
#
# 2003-2007 Ole Laursen <olau@hardworking.dk>
#
# XFCE4 modifications copyright:
#
# 2013, 2016-2018 OmegaPhil (OmegaPhil@startmail.com)

# Defining libraries to be built and their installation destination
plugindir = $(libdir)/xfce4/panel-plugins
plugin_LTLIBRARIES = libmxdock.la


# List of source files needed to build the library
libmxdock_la_SOURCES = \
						_gresources.c \
						Theme.cpp Theme.hpp \
						AppInfos.cpp AppInfos.hpp \
						Settings.cpp Settings.hpp \
						Group.cpp Group.hpp \
						GroupMenu.cpp GroupMenu.hpp \
						GroupMenuItem.cpp GroupMenuItem.hpp \
						GroupWindow.cpp GroupWindow.hpp \
						Helpers.cpp Helpers.hpp \
						Plugin.cpp Plugin.hpp \
						Dock.cpp Dock.hpp \
						Wnck.cpp Wnck.hpp \
						SettingsDialog.cpp SettingsDialog.hpp \
						register.c

libmxdock_la_LDFLAGS = \
                      -avoid-version \
                      -module \
                      -no-undefined \
    -export-symbols-regex '^xfce_panel_module_(preinit|init|construct)'


libmxdock_la_CPPFLAGS = \
                        @LIBXFCE4PANEL_CFLAGS@ \
                        @LIBXFCE4UI_CFLAGS@

libmxdock_la_LIBADD = \
                        @LIBXFCE4PANEL_LIBS@ \
                        @LIBXFCE4UI_LIBS@ \
                        $(GTK_LIBS) \
						$(CAIRO_LIBS) \
						$(WNCK_LIBS)

_gresources.c: _gresources.xml _dialogs.xml
	glib-compile-resources --generate-source --target _gresources.c _gresources.xml

pluginsdir = $(datadir)/xfce4/panel/plugins
plugins_in_files = mxdock.desktop.in
plugins_DATA = $(plugins_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = $(plugins_in_files)

# Recently (>=2017) dependent libraries seem to require building with
# the C++11 standard (see https://bugzilla.xfce.org/show_bug.cgi?id=13717)
# - this results in errors with GCC 5 and below when building with
# default configuration - 4.9.2 is bundled with Debian Jessie (2015),
# presumably when 2020 comes around I can consider addition of the std
# flag deprecated
AM_CXXFLAGS = -DWNCK_I_KNOW_THIS_IS_UNSTABLE $(GTK_CFLAGS) $(CAIRO_CFLAGS) $(WNCK_CFLAGS) $(SENSORS_CFLAGS) --std=c++11 -DPACKAGE_LOCALE_DIR=\"$(localedir)\"

CLEANFILES = _gresources.c
DISTCLEANFILES = $(NULL)
