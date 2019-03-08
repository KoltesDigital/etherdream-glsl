#include "FileWatcher.hpp"

void FileWatcher::watchFile(const std::string &path, callback_t callback)
{
	auto namePair = splitDirectoryNameAndBaseName(path);
	if (std::get<0>(namePair).empty())
	{
		return;
	}

	auto watchID = internalFileWatcher.addWatch(std::get<0>(namePair), &listener, false);

	listener.descriptorByWatchIDs.emplace(watchID, std::make_tuple(std::get<1>(namePair), callback));
}

void FileWatcher::start()
{
	internalFileWatcher.watch();

	for (auto it : listener.descriptorByWatchIDs)
	{
		std::get<1>(it.second)();
	}
}

void FileWatcher::Listener::handleFileAction(efsw::WatchID watchID, const std::string &, const std::string &filename, efsw::Action, std::string)
{
	auto it = descriptorByWatchIDs.find(watchID);
	if (it == std::end(descriptorByWatchIDs))
	{
		return;
	}

	if (std::get<0>(it->second) != filename)
	{
		return;
	}

	std::get<1>(it->second)();
}
