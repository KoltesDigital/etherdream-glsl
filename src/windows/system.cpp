#include "../common/system.hpp"

#include <memory>
#include <windows.h>

std::tuple<std::string, std::string> splitDirectoryNameAndBaseName(const std::string &path)
{
	auto bufferLength = 4096;
	auto fullPathBuffer = std::unique_ptr<char[]>(new char[bufferLength]);
	char *filename;

	auto fullPathLength = GetFullPathNameA(path.c_str(), bufferLength, fullPathBuffer.get(), &filename);
	if (!fullPathLength)
	{
		return std::make_tuple(std::string{}, std::string{});
	}

	std::string fullPath{ fullPathBuffer.get(), fullPathLength };

	auto lastSlash = fullPath.rfind("\\");
	if (lastSlash == std::string::npos)
	{
		return std::make_tuple(std::string{}, std::string{});
	}

	return std::make_tuple(fullPath.substr(0, lastSlash + 1), std::string{ filename });
}

void pause(float duration)
{
	Sleep((int)(duration * 1e3));
}
