#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace FileUtils
{
    std::string ReadText(const std::filesystem::path& filepath);

    void WriteLines(const std::filesystem::path& filepath, const std::vector<std::string>& lines);
}
