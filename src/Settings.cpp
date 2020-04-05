
#include "Settings.hpp"

namespace Settings
{

void handler(GtkWidget* dialog, gint response, PluginContext* context)
{
	g_object_set_data(G_OBJECT(context->plugin), "dialog", NULL);

	xfce_panel_plugin_unblock_menu(context->plugin);

	context->config->save();

	gtk_widget_destroy(dialog);
}

void launch(XfcePanelPlugin* plugin, PluginContext* context)
{
	GtkWidget* dialog;

	xfce_panel_plugin_block_menu(plugin);

	dialog = xfce_titled_dialog_new_with_buttons(_("Docklike"),
			GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(plugin))),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			"gtk-help", GTK_RESPONSE_HELP,
			"gtk-close", GTK_RESPONSE_OK,
			NULL);

	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

	gtk_window_set_icon_name(GTK_WINDOW(dialog), "xfce4-settings");

	g_object_set_data(G_OBJECT(plugin), "dialog", dialog);

	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(handler), context);

	gtk_widget_show(dialog);

}


}



