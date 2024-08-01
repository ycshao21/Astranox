#include "pch.hpp"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/async.h"
#include "spdlog/spdlog.h"

#include <iostream>

namespace Astranox
{
    void Logging::init()
    {
        // Initialize spdlog
        std::string pattern = "%^[%T] %n: %v%$";

        s_CoreLogger = std::shared_ptr<spdlog::logger>(spdlog::stdout_color_mt<spdlog::async_factory>("Core"));
        s_CoreLogger->set_pattern(pattern);
        s_CoreLogger->set_level(spdlog::level::trace);


        s_ClientLogger = std::shared_ptr<spdlog::logger>(spdlog::stdout_color_mt<spdlog::async_factory>("Astranox"));
        s_ClientLogger->set_pattern(pattern);
        s_ClientLogger->set_level(spdlog::level::trace);
    }

    void Logging::destroy()
    {
        s_CoreLogger.reset();
        s_ClientLogger.reset();

        spdlog::shutdown();
    }
}
