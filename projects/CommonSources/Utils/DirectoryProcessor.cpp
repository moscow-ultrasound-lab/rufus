#include "pre.h"

#include "DirectoryProcessor.h"
#include <XRADSystem/System.h>

XRAD_BEGIN

//--------------------------------------------------------------

namespace
{

void ProcessDirectoriesHelper(const string &path, const string &relative_path, size_t depth,
		double progress_start, double progress_range,
		function<void (const string &, const string &, double, double)> &func)
{
	if (!depth)
	{
		func(path, relative_path, progress_start, progress_range);
		return;
	}
	vector<string> tmp_files;
	vector<string> folders;
	GetDirectoryContent(tmp_files, folders, path);
	for (size_t i = 0; i < folders.size(); ++i)
	{
		const string &subfolder = folders[i];
		string new_relative_path = MergePath(relative_path, subfolder);
		string new_path = MergePath(path, subfolder);
		ProcessDirectoriesHelper(new_path, new_relative_path, depth - 1,
				progress_start + progress_range * double(i)/folders.size(),
				progress_range/folders.size(),
				func);
	}
}

} // namespace

//--------------------------------------------------------------

void ProcessDirectories(const string &path, const DirectoryProcessorOptions &options,
		function<void (const string &, const string &, double, double)> func)
{
	ProcessDirectoriesHelper(path, "", options.depth, 0, 1, func);
}

//--------------------------------------------------------------

XRAD_END
