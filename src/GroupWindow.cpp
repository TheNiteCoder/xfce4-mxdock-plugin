// ** opensource.org/licenses/GPL-3.0

#include "GroupWindow.hpp"

GroupWindow::GroupWindow(WnckWindow* wnckWindow)
{
	mWnckWindow = wnckWindow;
	mGroupMenuItem = new GroupMenuItem(this);

	WnckWorkspace* workspace = wnck_window_get_workspace(mWnckWindow);
	mWorkspaceID = wnck_workspace_get_number(workspace);

	mXID = wnck_window_get_xid(mWnckWindow);

	std::string groupName = Wnck::getGroupName(this); //check here for exotic group association (like libreoffice)
	mAppInfo = AppInfos::search(groupName);

	std::cout << "SEARCHING GROUPNAME:" << groupName << std::endl;
	if (mAppInfo == nullptr)
		std::cout << "NO MATCH:" << 0 << std::endl;
	else
	{
		std::cout << "> APPINFO NAME:" << mAppInfo->name << std::endl;
		std::cout << "> APPINFO PATH:" << mAppInfo->path << std::endl;
		std::cout << "> APPINFO ICON:" << mAppInfo->icon << std::endl;
	}

	// signal connection
	g_signal_connect(G_OBJECT(mWnckWindow), "name-changed",
		G_CALLBACK(+[](WnckWindow* window, GroupWindow* me) {
			me->mGroupMenuItem->updateLabel();
		}),
		this);

	g_signal_connect(G_OBJECT(mWnckWindow), "icon-changed",
		G_CALLBACK(+[](WnckWindow* window, GroupWindow* me) {
			me->mGroupMenuItem->updateIcon();
		}),
		this);

	g_signal_connect(G_OBJECT(mWnckWindow), "state-changed",
		G_CALLBACK(+[](WnckWindow* window, WnckWindowState changed_mask,
						WnckWindowState new_state, GroupWindow* me) {
			me->updateState(new_state, changed_mask);
		}),
		this);

	g_signal_connect(G_OBJECT(mWnckWindow), "workspace-changed",
		G_CALLBACK(+[](WnckWindow* window, GroupWindow* me) {
			WnckWorkspace* workspace = wnck_window_get_workspace(me->mWnckWindow);
			me->mWorkspaceID = wnck_workspace_get_number(workspace);
			me->mGroup->mWindowsCount.updateState();
			me->mGroup->mWindowsCount.forceFeedback();
		}),
		this);

	g_signal_connect(G_OBJECT(mWnckWindow), "geometry-changed",
		G_CALLBACK(+[](WnckWindow* window, GroupWindow* me) {
			me->mGroup->mWindowsCount.updateState();
			me->mGroup->mWindowsCount.forceFeedback();
			me->mGroup->checkWindowStates();
		}),
		this);
	std::cerr << "About to get into group" << std::endl;
	getInGroup(Dock::prepareGroup(mAppInfo));
	std::cerr << "Got into group" << std::endl;

	//initial state
	updateState(Wnck::getState(this));

	mGroupMenuItem->updateIcon();
	mGroupMenuItem->updateLabel();

	mGroup->mWindowsCount.updateState();
	std::cerr << "Reached end of GroupWindow::GroupWindow" << std::endl;
}

GroupWindow::~GroupWindow()
{
	leaveGroup(mGroup);
	delete mGroupMenuItem;
}

void GroupWindow::getInGroup(Group* group)
{
	mGroup = group;
	mGroup->add(this);
}

void GroupWindow::leaveGroup(Group* group) { group->remove(this); }

void GroupWindow::onActivate()
{
	Help::Gtk::cssClassAdd(GTK_WIDGET(mGroupMenuItem->mItem), const_cast<char*>("active"));
	gtk_widget_queue_draw(GTK_WIDGET(mGroupMenuItem->mItem));

	mGroup->onWindowActivate(this);
}

void GroupWindow::onUnactivate()
{
	Help::Gtk::cssClassRemove(GTK_WIDGET(mGroupMenuItem->mItem), const_cast<char*>("active"));
	gtk_widget_queue_draw(GTK_WIDGET(mGroupMenuItem->mItem));

	if (mGroup != nullptr)
		mGroup->onWindowUnactivate();
}

bool GroupWindow::getState(WnckWindowState flagMask)
{
	return (mState & flagMask) != 0;
}

void GroupWindow::activate(guint32 timestamp)
{
	Wnck::activate(this, timestamp);
}

void GroupWindow::minimize()
{
	Wnck::minimize(this);
}

void GroupWindow::showMenu() {}

void GroupWindow::updateState(ushort state, ushort changeMask)
{
	mState = state;

	if (changeMask & WnckWindowState::WNCK_WINDOW_STATE_SKIP_TASKLIST)
	{
		mGroup->mWindowsCount.updateState();

		if (state & WnckWindowState::WNCK_WINDOW_STATE_SKIP_TASKLIST)
			gtk_widget_hide(GTK_WIDGET(mGroupMenuItem->mItem));
		if (!mGroup->windowMeetsCriteria(this))
			gtk_widget_hide(GTK_WIDGET(mGroupMenuItem->mItem));
		else
			gtk_widget_show(GTK_WIDGET(mGroupMenuItem->mItem));
	}
}

bool GroupWindow::inCurrentWorkspace()
{
	return Wnck::inCurrentWorkspace(this);
}

bool GroupWindow::onCurrentMonitor()
{
	GdkDisplay* display = gdk_display_get_default();
	GdkWindow* dock = gtk_widget_get_window(GTK_WIDGET(Plugin::mXfPlugin));
	if (!GDK_IS_WINDOW(dock))
	{
		return true;
	}
	gint x, y, w, h;
	wnck_window_get_geometry(mWnckWindow, &x, &y, &w, &h);
	return gdk_display_get_monitor_at_window(display, dock) ==
		gdk_display_get_monitor_at_point(display, x + (w / 2), y + (h / 2));
}

bool GroupWindow::visible()
{
	return inCurrentWorkspace();
}

bool GroupWindow::meetsCriteria()
{
	return mGroup->windowMeetsCriteria(this);
}
