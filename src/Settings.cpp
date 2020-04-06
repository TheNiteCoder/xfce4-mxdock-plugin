
#include "Settings.hpp"

#include <iostream>

namespace Settings
{

void handler(GtkWidget* dialog, gint response, PluginContext* context)
{
	g_object_set_data(G_OBJECT(context->plugin), "dialog", NULL);

	xfce_panel_plugin_unblock_menu(context->plugin);

	context->config->save();

	gtk_widget_destroy(dialog);
}

void toggle_of_show_only_workspace_windows(GtkToggleButton* toggleButton, PluginContext* context)
{
	std::cerr << "Toggle button active: " << gtk_toggle_button_get_active(toggleButton) << std::endl;
	context->config->setShowOnlyWindowsInCurrentWorkspace(gtk_toggle_button_get_active(toggleButton));
}

void launch(XfcePanelPlugin* plugin, PluginContext* context)
{
	GtkWidget* dialog;

	xfce_panel_plugin_block_menu(plugin);

	dialog = xfce_titled_dialog_new_with_buttons(_("Docklike"),
			GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(plugin))),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			"gtk-help", GTK_RESPONSE_HELP, // Help button does nothing yet
			"gtk-close", GTK_RESPONSE_OK,
			NULL);

	GtkWidget* showOnlyWorkspaceWindowsCheckbox = gtk_check_button_new_with_label("Show only windows from current workspace");

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(showOnlyWorkspaceWindowsCheckbox), context->config->getShowOnlyWindowsInCurrentWorkspace());

	GtkBox* contents = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
	gtk_box_pack_start(contents, GTK_WIDGET(showOnlyWorkspaceWindowsCheckbox), true, true, 0);

	g_signal_connect(G_OBJECT(showOnlyWorkspaceWindowsCheckbox), "toggled", G_CALLBACK(toggle_of_show_only_workspace_windows), context);

	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

	gtk_window_set_icon_name(GTK_WINDOW(dialog), "xfce4-settings");

	g_object_set_data(G_OBJECT(plugin), "dialog", dialog);

	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(handler), context);

	gtk_widget_show_all(dialog);

}


}



