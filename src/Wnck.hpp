// ** opensource.org/licenses/GPL-3.0

#ifndef WNCK_HPP
#define WNCK_HPP

#include <map>
#include <fcntl.h>

#include <libwnck/libwnck.h>

#include "GroupWindow.hpp"
#include "Store.tpp"
#include "Helpers.hpp"
#include "Plugin.hpp"

class GroupWindow;

namespace Wnck
{
		void init();

		class WindowInfo
		{
		public:
			WindowInfo(WnckWindow* wnckWindow);
			~WindowInfo();

			gulong mXID;
			GroupWindow* mGroupWindow;
			bool mVisible;

		};

		gulong getActiveWindowXID();

		std::string getName(GroupWindow* groupWindow);
		std::string getGroupName(GroupWindow* groupWindow);
		gushort getState(GroupWindow* groupWindow);
		GdkPixbuf* getMiniIcon(GroupWindow* groupWindow);

		GtkWidget* getActionMenu(GroupWindow* groupWindow);


		void close(GroupWindow* groupWindow, guint32 timestamp);
		void activate(GroupWindow* groupWindow, guint32 timestamp);
		void minimize(GroupWindow* groupWindow);

		void setActiveWindow();
	
		bool windowInCurrentWorkspace(WnckWindow* window);

		extern WnckScreen* mWnckScreen;
		extern std::list<WindowInfo*> mWindows;
}

#endif
