#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4ui/libxfce4ui.h>

#include "PluginContext.hpp"

namespace Settings
{

void handler(GtkWidget* dialog, gint response, PluginContext* context);
void launch(XfcePanelPlugin* plugin, PluginContext* context);

}


#endif // SETTINGS_HPP
