#pragma once

#include <string>
#include <tuple>

void systemStartTime();

float systemGetTime();

std::tuple<std::string, std::string> systemSplitDirectoryNameAndBaseName(const std::string &path);

void systemPause(float duration = 0.f);
