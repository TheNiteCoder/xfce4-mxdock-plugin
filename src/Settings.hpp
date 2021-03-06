#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <glib.h>

#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "Dock.hpp"
#include "Helpers.hpp"
#include "Plugin.hpp"
#include "State.tpp"

namespace Settings
{
	void init();

	void saveFile();

	enum IndicatorSide
	{
		Automatic,
		Top,
		Bottom,
		Left,
		Right
	};

	extern State<bool> forceIconSize;
	extern State<int> iconSize;
	extern State<bool> noWindowsListIfSingle;
	extern State<int> indicatorStyle;
	extern State<GdkRGBA*> indicatorColor;
	extern State<std::list<std::string>> pinnedAppList;
	extern State<bool> showOnlyWindowsInCurrentWorkspace;
	extern State<bool> reverseIndicatorSide;
	extern State<bool> showOnlyWindowsOnCurrentMoniter;
	extern State<int> indicatorSide;

}; // namespace Settings

#endif
