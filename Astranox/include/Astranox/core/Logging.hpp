#pragma once

namespace Astranox {

    class Logging
    {
    public:
        static void init();
        static void destroy();

        inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
        inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_ClientLogger; }
    private:
        inline static std::shared_ptr<spdlog::logger> s_CoreLogger = nullptr;
        inline static std::shared_ptr<spdlog::logger> s_ClientLogger = nullptr;
    };

}

#define ENABLE_LOGGING 1

#if ENABLE_LOGGING
    // -------------- Core Logging Macros --------------
    #define AW_CORE_DEBUG(...)    ::Astranox::Logging::getCoreLogger()->debug(__VA_ARGS__)
    #define AW_CORE_TRACE(...)    ::Astranox::Logging::getCoreLogger()->trace(__VA_ARGS__)
    #define AW_CORE_INFO(...)     ::Astranox::Logging::getCoreLogger()->info(__VA_ARGS__)
    #define AW_CORE_WARN(...)     ::Astranox::Logging::getCoreLogger()->warn(__VA_ARGS__)
    #define AW_CORE_ERROR(...)    ::Astranox::Logging::getCoreLogger()->error(__VA_ARGS__)
    #define AW_CORE_CRITICAL(...) ::Astranox::Logging::getCoreLogger()->critical(__VA_ARGS__)

    // -------------- Client Logging Macros --------------
    #define AW_DEBUG(...)         ::Astranox::Logging::getClientLogger()->debug(__VA_ARGS__)
    #define AW_TRACE(...)         ::Astranox::Logging::getClientLogger()->trace(__VA_ARGS__)
    #define AW_INFO(...)          ::Astranox::Logging::getClientLogger()->info(__VA_ARGS__)
    #define AW_WARN(...)          ::Astranox::Logging::getClientLogger()->warn(__VA_ARGS__)
    #define AW_ERROR(...)         ::Astranox::Logging::getClientLogger()->error(__VA_ARGS__)
    #define AW_CRITICAL(...)      ::Astranox::Logging::getClientLogger()->critical(__VA_ARGS__)
#else
    #define AW_CORE_DEBUG(...)
    #define AW_CORE_TRACE(...)
    #define AW_CORE_INFO(...)
    #define AW_CORE_WARN(...)
    #define AW_CORE_ERROR(...)
    #define AW_CORE_CRITICAL(...)

    #define AW_DEBUG(...)
    #define AW_TRACE(...)
    #define AW_INFO(...)
    #define AW_WARN(...)
    #define AW_ERROR(...)
    #define AW_CRITICAL(...)
#endif
