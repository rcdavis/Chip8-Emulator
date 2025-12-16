#pragma once

#include "spdlog/fmt/fmt.h"

namespace StringUtils {
	template <typename... Args>
	std::string Format(const char* const format, Args... args) {
		return spdlog::fmt_lib::format(format, std::forward<Args>(args)...);
	}
}
