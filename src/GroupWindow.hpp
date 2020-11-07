// ** opensource.org/licenses/GPL-3.0

#ifndef GROUPWINDOW_HPP
#define GROUPWINDOW_HPP

#include <gtk/gtk.h>
#include <libwnck/libwnck.h>

#include <iostream>

#include "AppInfos.hpp"
#include "Dock.hpp"
#include "Group.hpp"
#include "GroupMenuItem.hpp"
#include "Helpers.hpp"
#include "Wnck.hpp"

class GroupMenuItem;
class Group;

class GroupWindow
{
  public:
	GroupWindow(WnckWindow* wnckWindow);
	~GroupWindow();

	void lateInit();
	void getInGroup(Group* group);
	void leaveGroup(Group* group);

	void onActivate();
	void onUnactivate();

	bool getState(WnckWindowState flagMask);

	void activate(guint32 timestamp);
	void minimize();
	void showMenu();

	bool visible();

	bool inCurrentWorkspace();
	bool onCurrentMonitor();

	bool meetsCriteria();

	Group* mGroup = nullptr;

	// TODO disabled during upstream merge
	bool mVisible;
	WnckScreen* mScreen;
	AppInfo* mAppInfo;
	WnckWindow* mWnckWindow;
	GroupMenuItem* mGroupMenuItem;
	int mWorkspaceID;

	gulong mXID;

	GdkRectangle mPreviousGeometry;

	void updateState(unsigned short state, unsigned short changeMask = USHRT_MAX);
	unsigned short mState;
};

#endif
