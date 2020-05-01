// ** opensource.org/licenses/GPL-3.0

#include "Wnck.hpp"

#include <algorithm>

#define RETURN_IF(b) if(b)return;

namespace Wnck
{
	
	WindowInfo::WindowInfo(WnckWindow* wnckWindow)
	{
		mGroupWindow = new GroupWindow(wnckWindow);
		mXID = wnck_window_get_xid(wnckWindow);
		mVisible = true;
	}

	void WindowInfo::construct()
	{
		mGroupWindow->lateInit();
	}

	WindowInfo::~WindowInfo()
	{
		delete mGroupWindow;
	}

	WnckScreen* mWnckScreen;
	// Store::KeyStore<gulong, GroupWindow*> mGroupWindows;
	std::list<WindowInfo*> mWindows;

	namespace // private:
	{
		std::map<std::string, std::string> mGroupNameRename //ADDIT GroupName aliases
		= { {"soffice", "libreoffice"},
			{"radium_linux.bin", "radium"},
		};

		void groupNameTransform(std::string& groupName, WnckWindow* wnckWindow)
		{
			// Rename from table
			std::map<std::string, std::string>::iterator itRenamed;
			if ((itRenamed = mGroupNameRename.find(groupName)) != mGroupNameRename.end())
				groupName = itRenamed->second;

			// LibreOffice <- needs window name tracking
			/*BAD if(groupName == "libreoffice")
			{
					std::string winName = getName(wnckWindow);
					std::cout << "NAME:" << winName << std::endl;
					if(!winName.empty())
					{
							std::string name =
			Help::String::toLowercase(Help::String::getLastWord(winName)); if(name ==
			"calc" || name == "draw" || name == "impress" || name == "math") groupName
			= "libreoffice-" + name; else groupName = "libreoffice-writer";

							return;
					}
			}*/
        }

		std::string getGroupNameSys(WnckWindow* wnckWindow)
		{
			// Wnck method const char *
			const char* buf = wnck_window_get_class_group_name(wnckWindow);
			if (buf != NULL && buf[0] != '\0')
				return buf;
			buf = wnck_window_get_class_instance_name(wnckWindow);
			if (buf != NULL && buf[0] != '\0')
				return buf;

			// proc/{pid}/cmdline method
			char buffer[512];
			std::string path = "/proc/" + std::to_string(wnck_window_get_pid(wnckWindow)) + "/cmdline";
			int fd = open(path.c_str(), O_RDONLY);
			if (fd >= 0)
			{
				int nbr = read(fd, buffer, 512);
				::close(fd);

				char* exe = basename(buffer);

				if (strcmp(exe, "python") != 0) // ADDIT graphical interpreters here
					return exe;

				char* it = buffer;
				while (*it++)
					;

				if (it < buffer + nbr)
					return basename(it);
			}

			// fallback : return window's name
			return wnck_window_get_name(wnckWindow);
		}
	} // namespace

	// public:

	void init()
	{
		mWnckScreen = wnck_screen_get_default();
		wnck_screen_force_update(mWnckScreen);

		// signal connection
		g_signal_connect(G_OBJECT(mWnckScreen), "window-opened",
		G_CALLBACK(+[](WnckScreen* screen, WnckWindow* wnckWindow)
		{
			WindowInfo* ptr = new WindowInfo(wnckWindow);
			mWindows.push_back(ptr);
			ptr->construct(); // construct WindowInfo after it has been added to the array
		}), NULL);

		g_signal_connect(G_OBJECT(mWnckScreen), "window-closed",
		G_CALLBACK(+[](WnckScreen* screen, WnckWindow* wnckWindow)
		{
			auto positer = std::find_if(mWindows.begin(), mWindows.end(), [=](WindowInfo* wi) {
				return wi->mXID == wnck_window_get_xid(wnckWindow);
			});
			if(positer == mWindows.end()) return;
			WindowInfo* ptr = *positer;
			mWindows.erase(positer);
			delete ptr;
		}), NULL);

		g_signal_connect(G_OBJECT(mWnckScreen), "active-window-changed",
			G_CALLBACK(+[](WnckScreen* screen, WnckWindow* previousActiveWindow) {
				setActiveWindow();
			}),
			NULL);

		// already opened windows
		for (GList* window_l = wnck_screen_get_windows(mWnckScreen);
			 window_l != NULL;
			 window_l = window_l->next)
		{
			WnckWindow* wnckWindow = WNCK_WINDOW(window_l->data);
			WindowInfo* inbetween = new WindowInfo(wnckWindow);
			mWindows.push_back(inbetween);
			inbetween->construct();
		}
		setActiveWindow();
	}

	gulong getActiveWindowXID()
	{
		WnckWindow* activeWindow = wnck_screen_get_active_window(mWnckScreen);
		if (!WNCK_IS_WINDOW(activeWindow))
			return NULL;

		return wnck_window_get_xid(activeWindow);
	}

	std::string getName(GroupWindow* groupWindow)
	{
		return wnck_window_get_name(groupWindow->mWnckWindow);
	}

	gushort getState(GroupWindow* groupWindow)
	{
		return wnck_window_get_state(groupWindow->mWnckWindow);
	}

	GdkPixbuf* getMiniIcon(GroupWindow* groupWindow)
	{
		return wnck_window_get_mini_icon(groupWindow->mWnckWindow);
	}

	void activate(GroupWindow* groupWindow, guint32 timestamp)
	{
		WnckWorkspace* workspace = wnck_window_get_workspace(groupWindow->mWnckWindow);
		if (workspace != NULL)
			wnck_workspace_activate(workspace, timestamp);
		wnck_window_activate(groupWindow->mWnckWindow, timestamp);
	}

	void close(GroupWindow* groupWindow, guint32 timestamp)
	{
		wnck_window_close(groupWindow->mWnckWindow, 0);
	}

	void minimize(GroupWindow* groupWindow)
	{
		wnck_window_minimize(groupWindow->mWnckWindow);
	}

	void setActiveWindow()
	{
		gulong activeXID = getActiveWindowXID();
		if (activeXID != NULL)
		{
			WindowInfo* info = *mWindows.begin();
			info->mGroupWindow->onUnactivate();
			auto iter = std::find_if(mWindows.begin(), mWindows.end(), [&activeXID](WindowInfo* wi){
				return wi->mXID == activeXID;
			});
			if(iter == mWindows.end())
			{
				std::cerr << "mxdock: failed to find an active window" << std::endl;
				return;
			}
			WindowInfo* ptr = *iter;
			mWindows.erase(iter);
			mWindows.push_front(ptr);
			ptr->mGroupWindow->onActivate();

			//mGroupWindows.first()->onUnactivate();
			//mGroupWindows.moveToStart(activeXID)->onActivate();
		}
	}

	bool windowInCurrentWorkspace(WnckWindow* window)
	{
		WnckWorkspace* currentWorkspace = wnck_screen_get_active_workspace(mWnckScreen);
	    if(currentWorkspace == NULL) return true;
		WnckWorkspace* windowWorkspace = wnck_window_get_workspace(window);
	    if(windowWorkspace == NULL) return true;
		int currentWorkspaceNumber = wnck_workspace_get_number(WNCK_WORKSPACE(currentWorkspace));
		int windowWorkspaceNumber = wnck_workspace_get_number(WNCK_WORKSPACE(windowWorkspace));
		return windowWorkspaceNumber == currentWorkspaceNumber;
	}


	std::string getGroupName(GroupWindow* groupWindow)
	{
		std::string groupName = Help::String::toLowercase(getGroupNameSys(groupWindow->mWnckWindow));
		groupNameTransform(groupName, groupWindow->mWnckWindow);

		return groupName;
	}

	GtkWidget* buildActionMenu(GroupWindow* groupWindow, Group* group)
	{
		GtkWidget* menu = (groupWindow != NULL) ? wnck_action_menu_new(groupWindow->mWnckWindow) : gtk_menu_new();

		AppInfo* appInfo = (groupWindow != NULL) ? groupWindow->mGroup->mAppInfo : group->mAppInfo;

		if (!appInfo->path.empty())
		{
			GtkWidget* launchAnother = gtk_menu_item_new_with_label((groupWindow != NULL) ? "Launch another" : "Launch");

			gtk_widget_show(launchAnother);

			gtk_menu_attach(GTK_MENU(menu), GTK_WIDGET(launchAnother), 0, 1, 0, 1);

			g_signal_connect(G_OBJECT(launchAnother), "activate",
				G_CALLBACK(+[](GtkMenuItem* menuitem, AppInfo* appInfo) {
					AppInfos::launch(appInfo);
				}),
				appInfo);

			if (group != NULL)
			{
				GtkWidget* separator = gtk_separator_menu_item_new();
				GtkWidget* pinToggle = gtk_menu_item_new_with_label(group->mPinned ? "Unpin" : "Pin");

				gtk_widget_show(separator);
				gtk_widget_show(pinToggle);

				gtk_menu_attach(GTK_MENU(menu), GTK_WIDGET(separator), 1, 2, 0, 1);
				gtk_menu_attach(GTK_MENU(menu), GTK_WIDGET(pinToggle), 1, 2, 0, 1);

				g_signal_connect(G_OBJECT(pinToggle), "activate",
					G_CALLBACK(+[](GtkMenuItem* menuitem, Group* group) {
						group->mPinned = !group->mPinned;
						if (!group->mPinned)
							group->updateStyle();
						Dock::savePinned();
					}),
					group);
			}

			if (group != NULL && group->mWindowsCount > 1)
			{
				GtkWidget* closeAll = gtk_menu_item_new_with_label("Close All");
				gtk_widget_show(closeAll);
				gtk_menu_shell_append(GTK_MENU_SHELL(menu), closeAll);

				g_signal_connect(G_OBJECT(closeAll), "activate",
					G_CALLBACK(+[](GtkMenuItem* menuitem, Group* group) {
						group->mWindows.forEach([](GroupWindow* w) -> void {
							Wnck::close(w, 0);
						});
					}),
					group);
			}

			return menu;
		}
	}
	
	int currentWorkspaceID()
	{
		WnckWorkspace* workspace = wnck_screen_get_active_workspace(mWnckScreen);
		return wnck_workspace_get_number(workspace);
	}

} // namespace Wnck
