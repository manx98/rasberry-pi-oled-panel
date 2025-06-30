//
// Created by wenyiyu on 2025/6/22.
//

#ifndef LOG_H
#define LOG_H
#include "spdlog/spdlog.h"
#define SPD_LOGGER(level, ...) get_logger()->log(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, level, __VA_ARGS__)
int log_init(const std::string &file, int max_size, int max_count);
std::shared_ptr<spdlog::logger> get_logger();
#define LOG_DEBUG(fmt, ...) SPD_LOGGER(spdlog::level::debug, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) SPD_LOGGER(spdlog::level::info, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) SPD_LOGGER(spdlog::level::warn, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) SPD_LOGGER(spdlog::level::err, fmt, ##__VA_ARGS__)
#endif //LOG_H
