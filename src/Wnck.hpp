// ** opensource.org/licenses/GPL-3.0

#ifndef WNCK_HPP
#define WNCK_HPP

#include <fcntl.h>
#include <libwnck/libwnck.h>

#include <map>

#include "Group.hpp" 
#include "GroupWindow.hpp"
#include "Helpers.hpp"
#include "Plugin.hpp"
#include "Store.tpp"

class GroupWindow;

namespace Wnck
{
		void init();

		class WindowInfo
		{
		public:
			WindowInfo(WnckWindow* wnckWindow);
			~WindowInfo();

			// this function is called after the new WindowInfo* is added to mWindows
			// this is to prevent from the electing of a new window taking place before 
			// the WindowInfo* is added to the array
			void construct();

			gulong mXID;
			GroupWindow* mGroupWindow;
			bool mVisible;

		};

		gulong getActiveWindowXID();

		std::string getName(GroupWindow* groupWindow);
		std::string getGroupName(GroupWindow* groupWindow);
		gushort getState(GroupWindow* groupWindow);
		GdkPixbuf* getMiniIcon(GroupWindow* groupWindow);

		void close(GroupWindow* groupWindow, guint32 timestamp);
		void activate(GroupWindow* groupWindow, guint32 timestamp);
		void minimize(GroupWindow* groupWindow);

		int currentWorkspaceID();

		void setActiveWindow();
	
		// bool windowInCurrentWorkspace(WnckWindow* window);

		GtkWidget* buildActionMenu(GroupWindow* groupWindow, Group* group);

		extern WnckScreen* mWnckScreen;
		extern std::list<WindowInfo*> mWindows;
}

#endif
