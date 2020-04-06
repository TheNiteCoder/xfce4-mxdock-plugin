// ** opensource.org/licenses/GPL-3.0

#include "Plugin.hpp"
#include "Helpers.hpp"

namespace Plugin
{
	XfcePanelPlugin* mXfPlugin;
	Config* mConfig;
	GdkDevice* mPointer;
	PluginContext* mContext;

	void init(XfcePanelPlugin* xfPlugin)
	{
		mXfPlugin = xfPlugin;

		mConfig = new Config(xfce_panel_plugin_save_location(mXfPlugin, true));

		mContext = plugin_context_new(mXfPlugin, mConfig);

		GdkDisplay* display = gdk_display_get_default();
		GdkDeviceManager* deviceManager = gdk_display_get_device_manager(display);
		mPointer = gdk_device_manager_get_client_pointer(deviceManager);

		AppInfos::init();

		Theme::init(gtk_widget_get_parent(GTK_WIDGET(mXfPlugin)));

		Dock::init();
		Wnck::init(mContext);

		//--------------------------------------------------

		gtk_container_add(GTK_CONTAINER(xfPlugin), GTK_WIDGET(Dock::mBox));

		//TODO orientation, settings, ...

		//--------------------------------------------------

		g_signal_connect(G_OBJECT(GTK_WIDGET(mXfPlugin)), "size-changed",
		G_CALLBACK(+[](XfcePanelPlugin *plugin, gint size){
			Dock::onPanelResize(size);
			return true;
		}), NULL);

		g_signal_connect(G_OBJECT(GTK_WIDGET(mXfPlugin)), "orientation-changed",
		G_CALLBACK(+[](XfcePanelPlugin *plugin, GtkOrientation orientation){
			Dock::onPanelOrientationChange(orientation);
		}), NULL);

		xfce_panel_plugin_menu_show_configure(mXfPlugin);
		g_signal_connect(G_OBJECT(mXfPlugin), "configure-plugin",
				G_CALLBACK(+[](XfcePanelPlugin* plugin, PluginContext* context){
					Settings::launch(plugin, context);
					}), mContext);
	}

	void getPointerPosition(gint* x, gint* y)
	{
		gdk_device_get_position(mPointer, NULL, x, y);
	}

}


//----------------------------------------------------------------------------------------------------------------------

extern "C" void construct(XfcePanelPlugin* xfPlugin)
{
	//xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");
	Plugin::init(xfPlugin);
}
