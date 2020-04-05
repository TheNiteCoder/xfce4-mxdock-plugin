
#include "PluginContext.hpp"


extern "C" PluginContext* plugin_context_new(XfcePanelPlugin* plugin, Config* config)
{
	PluginContext* ptr = new PluginContext;
	ptr->plugin = plugin;
	ptr->config = config;
	return ptr;
}

extern "C" void plugin_context_free(PluginContext* context)
{
	delete context;
}

