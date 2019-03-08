#pragma once

#include <string>
#include <tuple>

std::tuple<std::string, std::string> splitDirectoryNameAndBaseName(const std::string &path);

void pause(float duration = 0.f);
