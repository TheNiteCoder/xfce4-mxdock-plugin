#include "Group.hpp"

#include "Dock.hpp"
#include "GroupMenu.hpp"
#include "Settings.hpp"

#include "config.h"

static GtkTargetEntry entries[1] = {{const_cast<gchar*>("application/docklike_group"), 0, 0}};
static GtkTargetList* targetList = gtk_target_list_new(entries, 1);

Group::Group(AppInfo* appInfo, bool pinned) : mGroupMenu(this)
{
	mButton = gtk_button_new();
	gtk_style_context_add_class(gtk_widget_get_style_context(mButton), "group");
	gtk_style_context_add_class(gtk_widget_get_style_context(mButton), "flat");

	mIconPixbuf = nullptr;

	mAppInfo = appInfo;
	mPinned = pinned;
	mActive = false;

	mActiveBeforePressed = false;
	mTopWindow = nullptr;

	mSFocus = mSOpened = mSMany = mSHover = false;

	mWindowsCount.setup(
		0, [this]() -> uint {
			uint count = 0;
			mWindows.findIf([&count, this](GroupWindow* e) -> bool {
				if(e == nullptr)
				{
					std::cerr << "mxdock: found a nullptr GroupWindow*" << std::endl;
					return false;
				}
				if (!e->getState(WnckWindowState::WNCK_WINDOW_STATE_SKIP_TASKLIST))
				{
					if(e->meetsCriteria())
					{
						++count;
					}
					if(count == 2) return true;
				}
				return false;
			});
			return count; },
		[this](uint windowsCount) -> void {
			updateStyle();
			electNewTopWindow();
			if (windowsCount < 1 && !mPinned)
			{
				gtk_widget_hide(mButton);
			}
		});

	mLeaveTimeout.setup(40, [this]() {
		uint distance = mGroupMenu.getPointerDistance();

		if (distance >= mTolerablePointerDistance)
		{
			onMouseLeave();
			return false;
		}

		mTolerablePointerDistance -= 10;

		return true;
	});

	mMenuShowTimeout.setup(90, [this]() {
		onMouseEnter();
		return false;
	});

	g_signal_connect(
		G_OBJECT(mButton), "button-press-event",
		G_CALLBACK(+[](GtkWidget* widget, GdkEventButton* event, Group* me) {
			gdk_device_ungrab((event)->device, (event)->time);
			if (event->button != 3 && event->state & GDK_CONTROL_MASK)
			{
				std::cout << "STARTDRAG:state:" << event->state << std::endl;
				gtk_drag_begin_with_coordinates(widget, targetList, GDK_ACTION_MOVE, event->button, (GdkEvent*)event, -1, -1);
			}
			if (event->state & GDK_CONTROL_MASK)
			{
				me->mGroupMenu.hide();
				return false;
			}

			me->onButtonPress(event);
			return true;
		}),
		this);

	g_signal_connect(
		G_OBJECT(mButton), "button-release-event",
		G_CALLBACK(+[](GtkWidget* widget, GdkEventButton* event, Group* me) {
			if (event->button == 1) // 1 is left button
			{
				me->onButtonRelease(event);
				return true;
			}
			else if (event->button == 2) // 2 is middle button
			{
				me->closeAll();
				return true;
			}
			return false;
		}),
		this);

	g_signal_connect(
		G_OBJECT(mButton), "scroll-event",
		G_CALLBACK(+[](GtkWidget* widget, GdkEventScroll* event, Group* me) {
			me->onScroll((GdkEventScroll*)event);
			return true;
		}),
		this);

	g_signal_connect(G_OBJECT(mButton), "drag-begin",
		G_CALLBACK(+[](GtkWidget* widget, GdkDragContext* context, Group* me) {
			me->onDragBegin(context);
		}),
		this);

	g_signal_connect(G_OBJECT(mButton), "drag-motion",
		G_CALLBACK(+[](GtkWidget* widget, GdkDragContext* context, gint x, gint y, guint time, Group* me) {
			// return me->onDragMotion(widget, context, x, y, time);
			return me->onDragMotion(widget, context, x, y, time);
		}),
		this);

	g_signal_connect(G_OBJECT(mButton), "drag-leave",
		G_CALLBACK(+[](GtkWidget* widget, GdkDragContext* context, guint time, Group* me) {
			me->onDragLeave(context, time);
		}),
		this);

	g_signal_connect(G_OBJECT(mButton), "drag-data-get",
		G_CALLBACK(+[](GtkWidget* widget, GdkDragContext* context, GtkSelectionData* data, guint info, guint time, Group* me) {
			me->onDragDataGet(context, data, info, time);
		}),
		this);

	g_signal_connect(G_OBJECT(mButton), "drag-data-received",
		G_CALLBACK(+[](GtkWidget* widget, GdkDragContext* context, gint x, gint y, GtkSelectionData* data, guint info, guint time, Group* me) {
			me->onDragDataReceived(context, x, y, data, info, time);
		}),
		this);

	g_signal_connect(G_OBJECT(mButton), "enter-notify-event",
		G_CALLBACK(+[](GtkWidget* widget, GdkEventCrossing* event, Group* me) {
			if (!me->mActive && event->state & (GDK_BUTTON1_MASK))
			{
				me->mActiveBeforePressed = false;
				me->activate(event->time);
			}
			else if (me->mActive)
			{
				me->mActiveBeforePressed = true;
			}

			me->setStyle(Style::Hover, true);
			me->mLeaveTimeout.stop();
			me->mMenuShowTimeout.start();
			return false;
		}),
		this);

	g_signal_connect(
		G_OBJECT(mButton), "leave-notify-event",
		G_CALLBACK(+[](GtkWidget* widget, GdkEventCrossing* event, Group* me) {
			me->setStyle(Style::Hover, false);
			me->mMenuShowTimeout.stop();
			if (me->mPinned && me->mWindowsCount == 0)
				me->onMouseLeave();
			else
				me->setMouseLeaveTimeout();
			return true;
		}),
		this);

	g_signal_connect(
		G_OBJECT(mButton), "draw", G_CALLBACK(+[](GtkWidget* widget, cairo_t* cr, Group* me) {
			me->onDraw(cr);
			return false;
		}),
		this);

	g_signal_connect(G_OBJECT(Wnck::mWnckScreen), "active-workspace-changed",
		G_CALLBACK(+[](WnckScreen* screen, WnckWorkspace* prevWorkspace, Group* me) {
			me->mWindowsCount.updateState();
			me->mWindowsCount.forceFeedback();
		}),
		this);

	gtk_drag_dest_set(mButton, GTK_DEST_DEFAULT_DROP, entries, 1, GDK_ACTION_MOVE);

	if (mPinned)
		gtk_widget_show(mButton);

	g_object_set_data(G_OBJECT(mButton), "group", this);

	gtk_button_set_relief(GTK_BUTTON(mButton), GTK_RELIEF_NONE);

	gtk_widget_add_events(mButton, GDK_SCROLL_MASK);
	gtk_button_set_always_show_image(GTK_BUTTON(mButton), true);

	if (mAppInfo != nullptr && !mAppInfo->icon.empty())
	{
		GtkWidget* icon;

		if (mAppInfo->icon[0] == '/')
			mIconPixbuf = gdk_pixbuf_new_from_file(mAppInfo->icon.c_str(), nullptr);
		else
		{
			icon = gtk_image_new_from_icon_name(mAppInfo->icon.c_str(), GTK_ICON_SIZE_BUTTON);
			gtk_button_set_image(GTK_BUTTON(mButton), icon);
		}
	}
	else
	{
		GtkWidget* icon = gtk_image_new_from_icon_name("application-x-executable", GTK_ICON_SIZE_BUTTON);
		gtk_button_set_image(GTK_BUTTON(mButton), icon);
	}

	resize();
	checkWindowStates();
	mWindowsCount.updateState();
	mWindowsCount.forceFeedback();
}

void Group::add(GroupWindow* window)
{
	mWindows.push(window);

	mWindowsCount.updateState();

	mGroupMenu.add(window->mGroupMenuItem);

	mWindowsCount.updateState();
	mWindowsCount.forceFeedback();

	if (mWindowsCount == 1 && !mPinned)
	{
		gtk_box_reorder_child(GTK_BOX(Dock::mBox), GTK_WIDGET(mButton), -1);
	}
}

void Group::remove(GroupWindow* window)
{
	mWindows.pop(window);
	mWindowsCount.updateState();

	window->onUnactivate();

	mGroupMenu.remove(window->mGroupMenuItem);

	mWindowsCount.updateState();
	mWindowsCount.forceFeedback();

	setStyle(Style::Focus, false);
}

void Group::activate(guint32 timestamp)
{
	if (mWindowsCount == 0)
		return;

	GroupWindow* groupWindow = mTopWindow;

	if (groupWindow == nullptr)
	{
		electNewTopWindow();
		if (mTopWindow == nullptr)
			return;
		groupWindow = mTopWindow;
	}

	// 	mWindows.forEach([&timestamp, &groupWindow](GroupWindow* w) -> void {
	// 		if (w != groupWindow)
	// 			w->activate(timestamp);
	// 	});

	groupWindow->activate(timestamp);
}

void Group::resize()
{
	gtk_widget_set_size_request(mButton, (round((Dock::mPanelSize * 1.2) / 2) * 2), Dock::mPanelSize);

	GtkWidget* img;

	if (mIconPixbuf != nullptr)
	{
		GdkPixbuf* pixbuf = gdk_pixbuf_scale_simple(mIconPixbuf, Dock::mIconSize, Dock::mIconSize, GDK_INTERP_HYPER);
		GtkWidget* icon = gtk_image_new_from_pixbuf(pixbuf);
		gtk_button_set_image(GTK_BUTTON(mButton), icon);
		img = gtk_button_get_image(GTK_BUTTON(mButton));
	}
	else
	{
		img = gtk_button_get_image(GTK_BUTTON(mButton));
		gtk_image_set_pixel_size(GTK_IMAGE(img), Dock::mIconSize);
	}

	gtk_widget_set_valign(img, GTK_ALIGN_CENTER);
}

void Group::redraw()
{
	gtk_widget_queue_draw(mButton);
}

void Group::setStyle(Style style, bool val)
{
	switch (style)
	{
	case Style::Focus:
	{
		if (mSFocus != val)
		{
			mSFocus = val;
			gtk_widget_queue_draw(mButton);
		}
		break;
	}
	case Style::Opened:
	{
		if (mSOpened != val)
		{
			mSOpened = val;
			gtk_widget_queue_draw(mButton);
		}
		break;
	}
	case Style::Many:
	{
		if (mSMany != val)
		{
			mSMany = val;
			gtk_widget_queue_draw(mButton);
		}
		break;
	}
	case Style::Hover:
	{
		if (mSHover != val)
		{
			mSHover = val;
			gtk_widget_queue_draw(mButton);
		}
		break;
	}
	}
}

void Group::onDraw(cairo_t* cr)
{
	double aBack = 0.0;

	DockPosition dockPosition = mDockPosition;
	if (dockPosition == DockPosition::Floating || dockPosition == DockPosition::Empty)
	{
		dockPosition = DockPosition::Bottom;
	}
	if (Settings::indicatorSide == Settings::IndicatorSide::Left)
	{
		dockPosition = DockPosition::Left;
	}
	else if (Settings::indicatorSide == Settings::IndicatorSide::Right)
	{
		dockPosition = DockPosition::Right;
	}
	else if (Settings::indicatorSide == Settings::IndicatorSide::Top)
	{
		dockPosition = DockPosition::Top;
	}
	else if (Settings::indicatorSide == Settings::IndicatorSide::Bottom)
	{
		dockPosition = DockPosition::Bottom;
	}

	if (Settings::reverseIndicatorSide && Settings::indicatorSide == Settings::IndicatorSide::Automatic)
	{
		if (dockPosition == DockPosition::Left)
			dockPosition = DockPosition::Right;
		else if (dockPosition == DockPosition::Right)
			dockPosition = DockPosition::Left;
		else if (dockPosition == DockPosition::Bottom)
			dockPosition = DockPosition::Top;
		else if (dockPosition == DockPosition::Top)
			dockPosition = DockPosition::Bottom;
	}

	if (mSHover || mSFocus)
		aBack = 0.5;
	if (mSHover && mSFocus)
		aBack = 0.8;

	int w = gtk_widget_get_allocated_width(GTK_WIDGET(mButton));
	int h = gtk_widget_get_allocated_height(GTK_WIDGET(mButton));

	if (aBack > 0)
	{
		// Drawing the main box that highlights the box
		cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, aBack);
		cairo_rectangle(cr, 0, 0, w, h);
		cairo_fill(cr);
	}

	if (Settings::indicatorStyle == 0) // Bar
	{
		if (mSOpened)
		{
			// drawing status bar indicating a window is open
			if (mSFocus)
				cairo_set_source_rgba(cr, (*Settings::indicatorColor).red, (*Settings::indicatorColor).green, (*Settings::indicatorColor).blue, 1);
			else
				cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 1);

			if (dockPosition == DockPosition::Right)
			{
				cairo_rectangle(cr, w * 0.9231, 0, w, h);
			}
			else if (dockPosition == DockPosition::Left)
			{
				cairo_rectangle(cr, 0, 0, w * 0.0769, h);
			}
			else if (dockPosition == DockPosition::Top)
			{
				cairo_rectangle(cr, 0, 0, w, h * 0.0769);
			}
			else // if(dockPosition == DockPosition::Bottom)
			{
				cairo_rectangle(cr, 0, h * 0.9231, w, h);
			}

			cairo_fill(cr);

			// handle having an extra blip if there are serveral windows in group
			//if (mSMany && (mSOpened || mSHover))
			//{
			//if ((dockPosition == DockPosition::Right && !Settings::reverseIndicatorSide) ||
			//(dockPosition == DockPosition::Left && Settings::reverseIndicatorSide))
			//{
			//cairo_rectangle(cr, w * 0.9231, 0, w, h * 0.12);
			//}
			//else if ((dockPosition == DockPosition::Left && !Settings::reverseIndicatorSide) ||
			//(dockPosition == DockPosition::Right && Settings::reverseIndicatorSide))
			//{
			//cairo_rectangle(cr, 0, 0, w * 0.0679, h * 0.12);
			//}
			//cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.45);
			//cairo_fill(cr);
			//}
		}

		if (mSMany && (mSOpened || mSHover))
		{
			int x1, x2;
			cairo_pattern_t* pat;
			if ((dockPosition == DockPosition::Right) ||
				(dockPosition == DockPosition::Left))
			{
				x1 = 0;
				x2 = (int)h * 0.12;
				pat = cairo_pattern_create_linear(0, x1, 0, x2);
			}
			else
			{
				x1 = (int)w * 0.88;
				x2 = w;
				pat = cairo_pattern_create_linear(x1, 0, x2, 0);
			}

			if ((dockPosition == DockPosition::Right) ||
				(dockPosition == DockPosition::Left))
			{
				cairo_pattern_add_color_stop_rgba(pat, 0.7, 0, 0, 0, 0.15);
				cairo_pattern_add_color_stop_rgba(pat, 0.8, 0, 0, 0, 0.35);
				cairo_pattern_add_color_stop_rgba(pat, 1.0, 0, 0, 0, 0.45);
			}
			else
			{
				cairo_pattern_add_color_stop_rgba(pat, 0.0, 0, 0, 0, 0.45);
				cairo_pattern_add_color_stop_rgba(pat, 0.1, 0, 0, 0, 0.35);
				cairo_pattern_add_color_stop_rgba(pat, 0.3, 0, 0, 0, 0.15);
			}

			if (aBack > 0) // if hovering or active
			{
				if ((dockPosition == DockPosition::Right) ||
					(dockPosition == DockPosition::Left))
				{
					cairo_rectangle(cr, 0, x1, w, x2);
				}
				else
				{
					cairo_rectangle(cr, x1, 0, x2, h);
				}
			}
			else // if not hovering or active
			{
				if (dockPosition == DockPosition::Top)
				{
					cairo_rectangle(cr, x1, 0, x2, h * 0.0769);
				}
				else if (dockPosition == DockPosition::Bottom)
				{
					cairo_rectangle(cr, x1, h * 0.9231, x2, h);
				}
				else if (dockPosition == DockPosition::Right)
				{
					cairo_rectangle(cr, w * 0.9231, x1, w, x2);
				}
				else if (dockPosition == DockPosition::Left)
				{
					cairo_rectangle(cr, 0, x1, w * 0.0769, x2);
				}
				//else
				//{
				//cairo_rectangle(cr, x1, 0, x2, h);
				//}
			}
			//cairo_pattern_add_color_stop_rgba(pat, 0.0, 0, 0, 0, 0.45);
			//cairo_pattern_add_color_stop_rgba(pat, 0.1, 0, 0, 0, 0.35);
			//cairo_pattern_add_color_stop_rgba(pat, 0.3, 0, 0, 0, 0.15);

			//if (aBack > 0)
			//cairo_rectangle(cr, x1, 0, w, h);
			//else
			//cairo_rectangle(cr, x1, round(h * 0.9231), w, h);

			cairo_set_source(cr, pat);
			cairo_fill(cr);

			cairo_pattern_destroy(pat);
		}
	}
	else if (Settings::indicatorStyle == 1) // Dots
	{
		if (mSOpened)
		{
			double dotRadius = std::max(h * (0.093), 2.);
			double epos;
			if (dockPosition == DockPosition::Left)
				epos = w * 0.01;
			else if (dockPosition == DockPosition::Right)
				epos = w * 0.99;
			else if (dockPosition == DockPosition::Top)
				epos = h * 0.01;
			else
				epos = h * 0.99;

			double rgb[3] = {0, 1, 2};

			if (mSFocus)
			{
				rgb[0] = (*Settings::indicatorColor).red;
				rgb[1] = (*Settings::indicatorColor).green;
				rgb[2] = (*Settings::indicatorColor).blue;
			}
			else
			{
				rgb[0] = 0.7;
				rgb[1] = 0.7;
				rgb[2] = 0.7;
			}

			if (mSMany)
			{
				double pos;
				if (dockPosition == DockPosition::Right ||
					dockPosition == DockPosition::Left)
				{
					pos = (h / 2.) - dotRadius * 1;
				}
				else
				{
					pos = (w / 2.) - dotRadius * 1;
				}

				double x, y;
				if (dockPosition == DockPosition::Left ||
					dockPosition == DockPosition::Right)
				{
					x = epos;
					y = pos;
				}
				else
				{
					x = pos;
					y = epos;
				}
				cairo_pattern_t* pat = cairo_pattern_create_radial(x, y, 0, x, y, dotRadius);

				cairo_pattern_add_color_stop_rgba(pat, 0.3, rgb[0], rgb[1], rgb[2], 1);
				cairo_pattern_add_color_stop_rgba(pat, 1, rgb[0], rgb[1], rgb[2], 0.15);
				cairo_set_source(cr, pat);

				cairo_arc(cr, x, y, dotRadius, 0.0, 2.0 * M_PI);
				cairo_fill(cr);

				cairo_pattern_destroy(pat);

				if (dockPosition == DockPosition::Right ||
					dockPosition == DockPosition::Left)
				{
					pos = (h / 2.) + dotRadius * 1;
				}
				else
				{
					pos = (w / 2.) + dotRadius * 1;
				}

				if (dockPosition == DockPosition::Left ||
					dockPosition == DockPosition::Right)
				{
					x = epos;
					y = pos;
				}
				else
				{
					x = pos;
					y = epos;
				}
				pat = cairo_pattern_create_radial(x, y, 0, x, y, dotRadius);
				cairo_pattern_add_color_stop_rgba(pat, 0.3, rgb[0], rgb[1], rgb[2], 1);
				cairo_pattern_add_color_stop_rgba(pat, 1, rgb[0], rgb[1], rgb[2], 0.15);
				cairo_set_source(cr, pat);

				cairo_arc(cr, x, y, dotRadius, 0.0, 2.0 * M_PI);

				cairo_fill(cr);

				cairo_pattern_destroy(pat);
			}
			else
			{
				double pos;
				if (dockPosition == DockPosition::Right ||
					dockPosition == DockPosition::Left)
				{
					pos = h / 2.;
				}
				else
				{
					pos = w / 2.;
				}

				double x, y;
				if (dockPosition == DockPosition::Left ||
					dockPosition == DockPosition::Right)
				{
					x = epos;
					y = pos;
				}
				else
				{
					x = pos;
					y = epos;
				}
				cairo_pattern_t* pat = cairo_pattern_create_radial(x, y, 0, x, y, dotRadius);
				cairo_pattern_add_color_stop_rgba(pat, 0.3, rgb[0], rgb[1], rgb[2], 1);
				cairo_pattern_add_color_stop_rgba(pat, 1, rgb[0], rgb[1], rgb[2], 0.15);
				cairo_set_source(cr, pat);

				cairo_arc(cr, x, y, dotRadius, 0.0, 2.0 * M_PI);
				cairo_fill(cr);

				cairo_pattern_destroy(pat);
			}
		}
	}
}

void Group::onMouseEnter()
{
	mLeaveTimeout.stop();

	Dock::mGroups.forEach([this](std::pair<AppInfo*, Group*> g) -> void {
		if (&(g.second->mGroupMenu) != &(this->mGroupMenu))
			g.second->mGroupMenu.mGroup->onMouseLeave();
	});

	mGroupMenu.popup();

	this->setStyle(Style::Hover, true);
}

void Group::onMouseLeave()
{
	if (!mGroupMenu.mMouseHover)
	{
		this->setStyle(Style::Hover, false);
		mGroupMenu.hide();
	}
}

void Group::setMouseLeaveTimeout()
{
	mTolerablePointerDistance = 200;
	mLeaveTimeout.start();
}

void Group::updateStyle()
{
	// Hide menu items
	for (auto pair : mGroupMenu.mItemWindowPairs)
	{
		if (pair.second->mGroup->windowMeetsCriteria(pair.second) && !pair.second->getState(WnckWindowState::WNCK_WINDOW_STATE_SKIP_TASKLIST))
		{
			gtk_widget_show(GTK_WIDGET(pair.first));
		}
		else
		{
			gtk_widget_hide(GTK_WIDGET(pair.first));
		}
	}

	int wCount = mWindowsCount;

	if (mPinned || wCount)
		gtk_widget_show(mButton);
	else
		gtk_widget_hide(mButton);

	if (wCount)
		setStyle(Style::Opened, true);
	else
	{
		setStyle(Style::Opened, false);
		setStyle(Style::Focus, false);
	}

	if (wCount > 1)
		setStyle(Style::Many, true);
	else
		setStyle(Style::Many, false);
}

void Group::electNewTopWindow()
{
	if (mWindowsCount > 0)
	{
		GroupWindow* newTopWindow = nullptr;

		auto iter = std::find_if(Wnck::mWindows.begin(), Wnck::mWindows.end(), [this](GroupWindow* groupWindow) {
			g_assert(groupWindow != nullptr);
			return windowMeetsCriteria(groupWindow) && groupWindow->mGroup == this;
		});

		if (iter == Wnck::mWindows.end())
		{
			newTopWindow = nullptr;
		}
		else
		{
			GroupWindow* groupWindow = *iter;
			newTopWindow = groupWindow;
		}
		setTopWindow(newTopWindow);
	}
	else
	{
		setTopWindow(nullptr);
	}
}

void Group::onWindowActivate(GroupWindow* groupWindow)
{
	if (!windowMeetsCriteria(groupWindow))
		return;

	mActive = true;
	setStyle(Style::Focus, true);

	setTopWindow(groupWindow);
}

void Group::onWindowUnactivate()
{
	setStyle(Style::Focus, false);
	mActive = false;
}

void Group::setTopWindow(GroupWindow* groupWindow)
{
	if (groupWindow == nullptr)
		std::cerr << "mxdock: no new window for group " << mAppInfo->name << std::endl;
	mTopWindow = groupWindow;
}

void Group::onButtonPress(GdkEventButton* event)
{
	if (event->button != 3)
	{
		//std::cerr << "mxdock(DEBUG): Visible Windows Count: " << mWindowsCount << std::endl;
		return;
	}

	if (mWindowsCount == 0 || mTopWindow == nullptr)
	{
		GtkWidget* menu = gtk_menu_new();

		GtkWidget* launchAnother = gtk_menu_item_new_with_label("Launch");
		GtkWidget* separator = gtk_separator_menu_item_new();
		GtkWidget* pinToggle = mPinned ? gtk_menu_item_new_with_label("Unpin") : gtk_menu_item_new_with_label("Pin this app");

		gtk_widget_show(separator);
		gtk_widget_show(launchAnother);
		gtk_widget_show(pinToggle);

		gtk_menu_attach(GTK_MENU(menu), GTK_WIDGET(launchAnother), 0, 1, 0, 1);
		gtk_menu_attach(GTK_MENU(menu), GTK_WIDGET(separator), 1, 2, 0, 2);
		gtk_menu_attach(GTK_MENU(menu), GTK_WIDGET(pinToggle), 1, 2, 0, 2);

		g_signal_connect(G_OBJECT(launchAnother), "activate",
			G_CALLBACK(+[](GtkMenuItem* menuitem, Group* me) {
				AppInfos::launch(me->mAppInfo);
				me->mWindowsCount.updateState();
			}),
			this);

		g_signal_connect(G_OBJECT(pinToggle), "activate",
			G_CALLBACK(+[](GtkMenuItem* menuitem, Group* me) {
				me->mPinned = !me->mPinned;
				if (!me->mPinned)
					me->updateStyle();
				Dock::savePinned();
			}),
			this);

		gtk_menu_attach_to_widget(GTK_MENU(menu), GTK_WIDGET(mButton), nullptr);
		gtk_menu_popup_at_widget(GTK_MENU(menu), GTK_WIDGET(mButton), GDK_GRAVITY_SOUTH_WEST, GDK_GRAVITY_NORTH_WEST, (GdkEvent*)event);
	}
	else
	{
		if (mTopWindow != nullptr)
		{

			GtkWidget* menu = Wnck::buildActionMenu(mTopWindow, mTopWindow->mGroup);

			GtkWidget* launchAnother = gtk_menu_item_new_with_label("Launch another");
			GtkWidget* separator = gtk_separator_menu_item_new();
			GtkWidget* pinToggle = mPinned ? gtk_menu_item_new_with_label("Unpin") : gtk_menu_item_new_with_label("Pin this app");

			gtk_widget_show(separator);
			gtk_widget_show(launchAnother);
			gtk_widget_show(pinToggle);

			gtk_menu_attach(GTK_MENU(menu), GTK_WIDGET(launchAnother), 0, 1, 0, 1);
			gtk_menu_attach(GTK_MENU(menu), GTK_WIDGET(separator), 1, 2, 0, 2);
			gtk_menu_attach(GTK_MENU(menu), GTK_WIDGET(pinToggle), 1, 2, 0, 2);

			g_signal_connect(G_OBJECT(launchAnother), "activate",
				G_CALLBACK(+[](GtkMenuItem* menuitem, Group* me) {
					AppInfos::launch(me->mAppInfo);
					me->mWindowsCount.updateState();
					me->mWindowsCount.forceFeedback();
				}),
				this);

			g_signal_connect(G_OBJECT(pinToggle), "activate",
				G_CALLBACK(+[](GtkMenuItem* menuitem, Group* me) {
					me->mPinned = !me->mPinned;
					if (!me->mPinned)
						me->updateStyle();
					Dock::savePinned();
				}),
				this);

			gtk_menu_attach_to_widget(GTK_MENU(menu), GTK_WIDGET(mButton), nullptr);

			gtk_menu_popup_at_widget(GTK_MENU(menu), GTK_WIDGET(mButton), GDK_GRAVITY_SOUTH_WEST, GDK_GRAVITY_NORTH_WEST, (GdkEvent*)event);

			//then destroy TODO
			/* g_signal_connect (G_OBJECT (menu), "selection-done",
        	  G_CALLBACK (xfce_tasklist_button_menu_destroy), child);

						static void
				xfce_tasklist_button_menu_destroy (GtkWidget         *menu,
												XfceTasklistChild *child)
				{
				panel_return_if_fail (XFCE_IS_TASKLIST (child->tasklist));
				panel_return_if_fail (GTK_IS_TOGGLE_BUTTON (child->button));
				panel_return_if_fail (GTK_IS_WIDGET (menu));

				gtk_widget_destroy (menu);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (child->button), FALSE);
				}*/
		}
	}
}

void Group::onButtonRelease(GdkEventButton* event)
{
	if (event->state & GDK_SHIFT_MASK || (mPinned && mWindowsCount == 0))
	{
		AppInfos::launch(mAppInfo);
		mWindowsCount.updateState();
	}
	else if (mActive && mActiveBeforePressed)
	{
		if (mTopWindow != nullptr)
			mTopWindow->minimize();
	}
	else
	{
		if (mTopWindow == nullptr)
		{
			electNewTopWindow();
		}
		if (mTopWindow != nullptr)
		{
			guint32 timestamp = event->time;
			mTopWindow->activate(timestamp);
		}
		else
		{
			std::cerr << "mxdock: no top window for " << mAppInfo->name << std::endl;
		}
	}
}

void Group::onScroll(GdkEventScroll* event)
{
	if (mPinned && mWindowsCount == 0)
		return;

	if (!mActive)
	{
		if (mTopWindow != nullptr)
			mTopWindow->activate(event->time);
	}
	else
	{
		std::list<GroupWindow*> filtered = mWindows.underlying();
		filtered.remove_if([this](GroupWindow* wi) {
			return !wi->mGroup->windowMeetsCriteria(wi) || wi->mGroup != this;
		});

		if (filtered.size() == 0)
			return;

		auto current = std::find_if(filtered.begin(), filtered.end(), [=](GroupWindow* wi) {
			return wi == mTopWindow;
		});

		// If failed to find top window activate first one
		if (current == filtered.end())
		{
			filtered.front()->activate(event->time);
			setTopWindow(filtered.front());
		}

		if (event->direction == GDK_SCROLL_UP)
		{
			if (++current == filtered.end())
			{
				filtered.front()->activate(event->time);
				setTopWindow(filtered.front());
			}
			else
			{
				GroupWindow* window = (*current);
				window->activate(event->time);
				setTopWindow(window);
			}
		}
		else if (event->direction == GDK_SCROLL_DOWN)
		{
			if (--current == filtered.end())
			{
				filtered.back()->activate(event->time);
				setTopWindow(filtered.back());
			}
			else
			{
				GroupWindow* window = (*current);
				window->activate(event->time);
				setTopWindow(window);
			}
		}
	}
}

bool Group::onDragMotion(GtkWidget* widget, GdkDragContext* context, int x, int y, guint time)
{
	GdkModifierType mask;

	gdk_window_get_pointer(gtk_widget_get_window(widget), nullptr, nullptr, &mask);
	if (mask & GDK_CONTROL_MASK)
		gtk_drag_cancel(context);

	GList* tmp_list = gdk_drag_context_list_targets(context);
	if (tmp_list != nullptr)
	{
		char* name = gdk_atom_name(GDK_POINTER_TO_ATOM(tmp_list->data));
		std::string target = name;
		g_free(name);

		if (target != "application/docklike_group")
		{
			if (mWindowsCount > 0)
			{
				if (mTopWindow != nullptr)
				{
					mTopWindow->activate(time);

					if (!mGroupMenu.mVisible)
						onMouseEnter();
				}

				if (!mGroupMenu.mVisible)
					onMouseEnter();
			}

			gdk_drag_status(context, GDK_ACTION_DEFAULT, time);
			return true;
		}
	}

	gtk_style_context_add_class(gtk_widget_get_style_context(mButton), "drop");

	gdk_drag_status(context, GDK_ACTION_MOVE, time);
	return true;
}

void Group::onDragLeave(const GdkDragContext* context, guint time)
{
	gtk_style_context_remove_class(gtk_widget_get_style_context(mButton), "drop");
}

void Group::onDragDataGet(const GdkDragContext* context, GtkSelectionData* selectionData, guint info, guint time)
{
	Group* me = this;
	std::cout << "pme:" << me << std::endl;

	// TODO is the source object copied or passed by a pointer ?
	gtk_selection_data_set(selectionData, gdk_atom_intern("button", false), 32, (const guchar*)me, sizeof(gpointer) * 32);
}

void Group::onDragDataReceived(const GdkDragContext* context, int x, int y, const GtkSelectionData* selectionData,
	guint info, guint time)
{
	GdkAtom dt = gtk_selection_data_get_data_type(selectionData);
	// TODO figure out usage of this
	// if(gdk_atom_name(dt) == "button")

	Group* source = (Group*)gtk_selection_data_get_data(selectionData);
	Dock::moveButton(source, this);
}

void Group::onDragBegin(GdkDragContext* context)
{
	gtk_drag_set_icon_name(context, mAppInfo->icon.c_str(), 0, 0);
}

double Group::degreesToRadians(double degrees)
{
	return degrees * (M_PI / 180);
}

bool Group::windowMeetsCriteria(GroupWindow* window)
{
	bool result = true;
	if (Settings::showOnlyWindowsInCurrentWorkspace && Settings::showOnlyWindowsOnCurrentMoniter)
	{
		result = window->inCurrentWorkspace() && window->onCurrentMonitor();
	}
	else if (Settings::showOnlyWindowsInCurrentWorkspace)
	{
		result = window->inCurrentWorkspace();
	}
	else if (Settings::showOnlyWindowsOnCurrentMoniter)
	{
		result = window->onCurrentMonitor();
	}
	return result;
}

void Group::checkWindowStates()
{
	Wnck::setActiveWindow();
}

void Group::closeAll()
{
	mWindows.forEach([](GroupWindow* window) {
		if (window->getState(WnckWindowState::WNCK_WINDOW_STATE_SKIP_TASKLIST))
			return;
		Wnck::close(window, 0);
	});
	mWindowsCount.updateState();
	mWindowsCount.forceFeedback();
}
