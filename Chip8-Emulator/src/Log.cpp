#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

Ref<spdlog::logger> Log::s_Logger;

void Log::Init()
{
    std::vector<spdlog::sink_ptr> logSinks;
    logSinks.emplace_back(CreateRef<spdlog::sinks::stdout_color_sink_mt>());
    logSinks.emplace_back(CreateRef<spdlog::sinks::basic_file_sink_mt>("Chip8.log", true));

    logSinks[0]->set_pattern("%^[%T] %n: %v%$");
    logSinks[1]->set_pattern("[%T] [%l] %n: %v");

    s_Logger = CreateRef<spdlog::logger>("Chip8", std::begin(logSinks), std::end(logSinks));
    s_Logger->set_level(spdlog::level::trace);
    s_Logger->flush_on(spdlog::level::trace);
    spdlog::register_logger(s_Logger);
}
