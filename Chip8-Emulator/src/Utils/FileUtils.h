#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace FileUtils
{
    void WriteLines(const std::filesystem::path& filepath, const std::vector<std::string>& lines);
}
