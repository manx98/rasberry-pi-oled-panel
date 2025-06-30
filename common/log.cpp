//
// Created by wenyiyu on 2025/6/22.
//
#include "log.h"

#include <spdlog/sinks/rotating_file_sink.h>

static std::shared_ptr<spdlog::logger> my_logger;

int log_init(const std::string& file, int max_size, int max_count)
{
    if (!file.empty())
    {
        my_logger = spdlog::rotating_logger_mt("my_logger", file, max_size, max_count);
    }
}

std::shared_ptr<spdlog::logger> get_logger()
{
    if (my_logger)
    {
        return my_logger;
    }
    return spdlog::default_logger();
}
