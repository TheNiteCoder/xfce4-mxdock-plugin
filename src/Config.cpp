#include "Config.hpp"

Config::Config(std::string path)
{
	mPath = path;

	std::cout << "SAVEPATH:" << path << std::endl;

	mFile = g_key_file_new();
	g_key_file_load_from_file(mFile, mPath.c_str(), G_KEY_FILE_NONE, nullptr);
}

void Config::save()
{
	g_key_file_save_to_file(mFile, mPath.c_str(), nullptr);
}

void Config::setPinned(std::list<std::string> pinnedApps)
{
	std::vector<char*> buf;
	for (std::string& s : pinnedApps)
		buf.push_back(&s[0]);

	g_key_file_set_string_list(mFile, "user", "pinned", buf.data(), buf.size());
}

std::list<std::string> Config::getPinned()
{
	std::list<std::string> ret;
	gchar** clist = g_key_file_get_string_list(mFile, "user", "pinned", nullptr, nullptr);

	if (clist != nullptr)
		for (int i = 0; clist[i] != nullptr; ++i)
			ret.push_back(clist[i]);

	g_strfreev(clist);
	return ret;
}

void Config::setShowOnlyWindowsInCurrentWorkspace(bool value)
{
	g_key_file_set_boolean(mFile, "user", "showOnlyWindowsInCurrentWorkspace", value);
}

bool Config::getShowOnlyWindowsInCurrentWorkspace()
{
	gboolean gbool = g_key_file_get_boolean(mFile, "user", "showOnlyWindowsInCurrentWorkspace", nullptr);
	return (gbool == TRUE) ? true : false;
}
