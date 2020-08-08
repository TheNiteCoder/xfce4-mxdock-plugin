#include "Settings.hpp"

namespace Settings
{
	std::string mPath;
	GKeyFile* mFile;

	State<bool> forceIconSize;
	State<int> iconSize;
	State<bool> noWindowsListIfSingle;
	State<int> indicatorStyle;
	State<std::list<std::string>> pinnedAppList;
	State<bool> showOnlyWindowsInCurrentWorkspace;
	State<bool> reverseIndicatorSide;
	State<bool> showOnlyWindowsOnCurrentMoniter;

	void init()
	{
		mPath = xfce_panel_plugin_save_location(Plugin::mXfPlugin, true);

		mFile = g_key_file_new();
		g_key_file_load_from_file(mFile, mPath.c_str(), G_KEY_FILE_NONE, nullptr);

		forceIconSize.setup(g_key_file_get_boolean(mFile, "user", "forceIconSize", nullptr),
			[](bool forceIconSize) -> void {
				g_key_file_set_boolean(mFile, "user", "forceIconSize", forceIconSize);
				saveFile();

				Dock::onPanelResize();
			});

		iconSize.setup(g_key_file_get_integer(mFile, "user", "iconSize", nullptr),
			[](int iconSize) -> void {
				g_key_file_set_integer(mFile, "user", "iconSize", iconSize);
				saveFile();

				Dock::onPanelResize();
			});

		indicatorStyle.setup(g_key_file_get_integer(mFile, "user", "indicatorStyle", nullptr),
			[](int indicatorStyle) -> void {
				g_key_file_set_integer(mFile, "user", "indicatorStyle", indicatorStyle);
				saveFile();

				Dock::redraw();
			});

		noWindowsListIfSingle.setup(g_key_file_get_boolean(mFile, "user", "noWindowsListIfSingle", nullptr),
			[](bool noWindowsListIfSingle) -> void {
				g_key_file_set_boolean(mFile, "user", "noWindowsListIfSingle", noWindowsListIfSingle);
				saveFile();
			});

		showOnlyWindowsInCurrentWorkspace.setup(g_key_file_get_boolean(mFile, "user", "showOnlyWindowsInCurrentWorkspace", nullptr),
			[](bool showOnlyWindowsInCurrentWorkspace) -> void {
				g_key_file_set_boolean(mFile, "user", "showOnlyWindowsInCurrentWorkspace", showOnlyWindowsInCurrentWorkspace);
				saveFile();
			});
	
		reverseIndicatorSide.setup(g_key_file_get_boolean(mFile, "user", "reverseIndicatorSide", nullptr),
			[](bool reverseIndicatorSide) -> void {
				g_key_file_set_boolean(mFile, "user", "reverseIndicatorSide", reverseIndicatorSide);
				saveFile();
				Dock::redraw();
			});

		showOnlyWindowsOnCurrentMoniter.setup(g_key_file_get_boolean(mFile, "user", "showOnlyWindowsOnCurrentMoniter", nullptr),
			[](bool value) -> void {
				g_key_file_set_boolean(mFile, "user", "showOnlyWindowsOnCurrentMoniter", value);
				saveFile();
				Dock::redraw();
			});

		gchar** pinnedListBuffer = g_key_file_get_string_list(mFile, "user", "pinned", nullptr, nullptr);
		pinnedAppList.setup(Help::Gtk::bufferToStdStringList(pinnedListBuffer),
			[](std::list<std::string> list) -> void {
				std::vector<char*> buf = Help::Gtk::stdToBufferStringList(list);
				g_key_file_set_string_list(mFile, "user", "pinned", buf.data(), buf.size());
				saveFile();
			});
		g_strfreev(pinnedListBuffer);
	}

	void saveFile()
	{
		g_key_file_save_to_file(mFile, mPath.c_str(), nullptr);
	}
} // namespace Settings
