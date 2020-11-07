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
	void earlyInit();

	gulong getActiveWindowXID();

	std::string getName(GroupWindow* groupWindow);
	std::string getGroupName(GroupWindow* groupWindow);
	gushort getState(GroupWindow* groupWindow);
	GdkPixbuf* getMiniIcon(GroupWindow* groupWindow);

	void close(GroupWindow* groupWindow, guint32 timestamp);
	void activate(GroupWindow* groupWindow, guint32 timestamp);
	void minimize(GroupWindow* groupWindow);

	bool inCurrentWorkspace(GroupWindow* groupWindow);

	void updateWorkspaceID();

	void setActiveWindow();

	// bool windowInCurrentWorkspace(WnckWindow* window);

	GtkWidget* buildActionMenu(GroupWindow* groupWindow, Group* group);

	extern WnckScreen* mWnckScreen;
	extern std::list<GroupWindow*> mWindows;
	extern int mCurrentWorkspaceID;
} // namespace Wnck

#endif
