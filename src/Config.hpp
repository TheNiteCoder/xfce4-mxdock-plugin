#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>

#include <list>
#include <string>
#include <vector>

#include <glib.h>

class Config
{
  public:
	Config(std::string path);
	void save();

	void setPinned(std::list<std::string> pinnedApps);
	std::list<std::string> getPinned();

	void setShowOnlyWindowsInCurrentWorkspace(bool value);
	bool getShowOnlyWindowsInCurrentWorkspace();

  private:
	GKeyFile* mFile;
	std::string mPath;
};

#endif
