#include "FileUtils.h"

#include "Log.h"

#include <fstream>
#include <iterator>

namespace FileUtils
{
    std::string ReadText(const std::filesystem::path& filepath)
    {
        std::ifstream file(filepath);
        if (!file)
        {
            LOG_ERROR("Failed to open file at {}", filepath.string());
            return {};
        }

        const auto fileSize = std::filesystem::file_size(filepath);
        if (fileSize == 0)
            return {};

        std::string text;
        text.resize(fileSize);

        file.read(std::data(text), fileSize);

        return text;
    }

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
