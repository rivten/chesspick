#pragma once

#include <string>
#include <unordered_map>

void fcgi_start(std::string (*)(std::string, const std::unordered_map<std::string, std::string>&));

