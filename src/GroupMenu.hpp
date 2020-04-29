#ifndef DOCK_BUTTON_MENU_HPP
#define DOCK_BUTTON_MENU_HPP

#include <gtk/gtk.h>

#include <list>
#include <iostream>

class Group;
class GroupMenuItem;
class GroupWindow;

class GroupMenu
{
  public:
	GroupMenu(Group* dockButton);

	void add(GroupMenuItem* menuItem);
	void remove(GroupMenuItem* menuItem);

	void popup();
	void hide();

	uint getPointerDistance();

	Group* mGroup;

	std::list<std::pair<GtkEventBox*, GroupWindow*> > mItemWindowPairs;
	GtkWidget* mWindow;
	GtkWidget* mBox;

	bool mVisible;
	bool mMouseHover;
};

#endif

