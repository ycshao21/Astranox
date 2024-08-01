#pragma once
#include "Logging.hpp"

#define BIT(x) (1 << x)

#ifdef AW_CONFIG_DEBUG
    #define AW_ENABLE_ASSERTS
#endif

#ifdef AW_ENABLE_ASSERTS
    #define AW_ASSERT(x, ...) { if(!(x)) { AW_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
    #define AW_CORE_ASSERT(x, ...) { if(!(x)) { AW_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
    #define AW_ASSERT(x, ...)
    #define AW_CORE_ASSERT(x, ...)
#endif
