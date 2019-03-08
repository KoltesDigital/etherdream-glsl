#pragma once

#include <efsw/efsw.hpp>
#include <functional>
#include <string>
#include <unordered_map>

#include "system.hpp"

class FileWatcher
{
public:
	using callback_t = std::function<void()>;

	void watchFile(const std::string &path, callback_t callback);

	void start();

private:
	class Listener : public efsw::FileWatchListener
	{
	public:
		std::unordered_map<efsw::WatchID, std::tuple<std::string, callback_t>> descriptorByWatchIDs;

		void handleFileAction(efsw::WatchID watchID, const std::string &directory, const std::string &filename, efsw::Action action, std::string oldFilename = "") override;
	};

	efsw::FileWatcher internalFileWatcher;
	Listener listener;
};