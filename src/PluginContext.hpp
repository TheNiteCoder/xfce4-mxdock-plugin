#ifndef PLUGIN_CONTEXT_HPP
#define PLUGIN_CONTEXT_HPP

#include "Config.hpp"

extern "C" 
{

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>

G_BEGIN_DECLS

struct _PluginContext
{
	XfcePanelPlugin* plugin;
	Config* config;
};

typedef _PluginContext PluginContext;

PluginContext* plugin_context_new(XfcePanelPlugin* plugin, Config* config);

void plugin_context_free(PluginContext* context);

G_END_DECLS

}

#endif // PLUGIN_CONTEXT_HPP

