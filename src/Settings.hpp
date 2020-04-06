#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4ui/libxfce4ui.h>

#include "Plugin.hpp"

namespace Settings
{

void handler(GtkWidget* dialog, gint response);
void launch(XfcePanelPlugin* plugin);

}


#endif // SETTINGS_HPP
