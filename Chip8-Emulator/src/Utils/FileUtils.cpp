#include "FileUtils.h"

#include "Log.h"

#include <fstream>

namespace FileUtils
{
    void WriteLines(const std::filesystem::path& filepath, const std::vector<std::string>& lines)
    {
        std::ofstream file(filepath);
        if (!file)
        {
            LOG_ERROR("Failed to open file at {}", filepath.string());
            return;
        }

        for (const auto& line : lines)
            file << line << '\n';
    }
}
