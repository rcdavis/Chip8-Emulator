#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace FileUtils
{
    std::string ReadText(const std::filesystem::path& filepath);
    void WriteText(const std::filesystem::path& filepath, const std::string& text);

    void WriteLines(const std::filesystem::path& filepath, const std::vector<std::string>& lines);
}
