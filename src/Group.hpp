#ifndef DOCK_BUTTON_HPP
#define DOCK_BUTTON_HPP

#include <gtk/gtk.h>

#include <algorithm>
#include <iostream>
#include <math.h>
#include <memory>

#include "AppInfos.hpp"
#include "GroupMenu.hpp"
#include "GroupWindow.hpp"
#include "Helpers.hpp"
#include "Plugin.hpp"
#include "State.tpp"

class GroupWindow;

class Group
{
  public:
	enum DockPosition
	{
		Empty,
		Floating,
		Top,
		Bottom,
		Left,
		Right
	};

	enum Style
	{
		Focus,
		Opened,
		Many,
		Hover
	};

	Group(AppInfo* appInfo, bool pinned);
	~Group();

	void add(GroupWindow* window);
	void remove(GroupWindow* window);

	void resize();
	void redraw();
	void setStyle(Style style, bool val);
	void updateStyle();
	void electNewTopWindow();

	void onDraw(cairo_t* cr);

	void onWindowActivate(GroupWindow* groupWindow);
	void onWindowUnactivate();

	void onButtonPress(GdkEventButton* event);
	void onButtonRelease(GdkEventButton* event);
	void onScroll(GdkEventScroll* scroll_event);
	void onMouseEnter();
	void onMouseLeave();
	void setMouseLeaveTimeout();

	bool onDragMotion(GtkWidget* widget, GdkDragContext* context, int x, int y, guint time);
	void onDragLeave(const GdkDragContext* context, guint time);
	void onDragDataGet(const GdkDragContext* context, GtkSelectionData* selectionData, guint info, guint time);
	void onDragDataReceived(const GdkDragContext* context, int x, int y, const GtkSelectionData* selectionData, guint info, guint time);
	void onDragBegin(GdkDragContext* context);

	bool windowMeetsCriteria(GroupWindow* window);

	void activate(guint32 timestamp);

	void closeAll();

	bool mHover;
	bool mPinned;
	GtkWidget* mButton;

	GroupMenu mGroupMenu;
	bool mSFocus;
	bool mSOpened;
	bool mSMany;
	bool mSHover;
	bool mActiveBeforePressed;
	uint mTolerablePointerDistance;
	LogicalState<uint>
		mWindowsCount;

	AppInfo* mAppInfo;
	Store::List<GroupWindow*> mWindows;
	GroupWindow* mTopWindow;
	GdkPixbuf* mIconPixbuf;

	void setTopWindow(GroupWindow* groupWindow);

	void checkWindowStates();

	bool mActive;
	bool mDropHover;

	Help::Gtk::Timeout mLeaveTimeout;
	Help::Gtk::Timeout mMenuShowTimeout;

	GtkOrientation mDockOrientation;
	DockPosition mDockPosition;

	static double degreesToRadians(double degrees);
};

#endif
