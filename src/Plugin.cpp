// ** opensource.org/licenses/GPL-3.0

#include "Plugin.hpp"

#include "Helpers.hpp"

#include "config.h"

namespace Plugin
{
	XfcePanelPlugin* mXfPlugin;
	GdkDevice* mPointer;

	void init(XfcePanelPlugin* xfPlugin)
	{
		mXfPlugin = xfPlugin;

		GdkDisplay* display = gdk_display_get_default();
		GdkDeviceManager* deviceManager = gdk_display_get_device_manager(display);
		mPointer = gdk_device_manager_get_client_pointer(deviceManager);

		Settings::init();
		AppInfos::init();
		Wnck::earlyInit(); // creates mWnckScreen for Dock::init for connecting of signals
		Dock::init();
		Wnck::init();
		Theme::init();

		Dock::UpdateGroupsScreenPosition();

		//--------------------------------------------------

		gtk_container_add(GTK_CONTAINER(mXfPlugin), GTK_WIDGET(Dock::mBox));

		xfce_panel_plugin_menu_show_configure(mXfPlugin);

		//--------------------------------------------------

		g_signal_connect(G_OBJECT(GTK_WIDGET(mXfPlugin)), "size-changed",
			G_CALLBACK(+[](XfcePanelPlugin* plugin, gint size) {
				Dock::onPanelResize(size);
				return true;
			}),
			nullptr);

		g_signal_connect(G_OBJECT(GTK_WIDGET(mXfPlugin)), "orientation-changed",
			G_CALLBACK(+[](XfcePanelPlugin* plugin, GtkOrientation orientation) {
				Dock::onPanelOrientationChange(orientation);
			}),
			nullptr);

		g_signal_connect(G_OBJECT(GTK_WIDGET(mXfPlugin)), "screen-position-changed",
			G_CALLBACK(+[](XfcePanelPlugin* plugin, XfceScreenPosition pos) {
				Dock::onScreenPositionChange(pos);
			}),
			nullptr);

		g_signal_connect(G_OBJECT(mXfPlugin), "configure-plugin",
			G_CALLBACK(+[](XfcePanelPlugin* plugin) {
				SettingsDialog::popup();
			}),
			nullptr);
	}

	void getPointerPosition(gint* x, gint* y)
	{
		gdk_device_get_position(mPointer, nullptr, x, y);
	}

} // namespace Plugin

//----------------------------------------------------------------------------------------------------------------------

extern "C" void construct(XfcePanelPlugin* xfPlugin)
{
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
	textdomain(PACKAGE);
	Plugin::init(xfPlugin);
}
