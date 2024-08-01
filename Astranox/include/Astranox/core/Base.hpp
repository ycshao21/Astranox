#pragma once
#include "Logging.hpp"

#define BIT(x) (1 << x)

#ifdef AST_CONFIG_DEBUG
    #define AST_ENABLE_ASSERTS
#endif

#ifdef AST_ENABLE_ASSERTS
    #define AST_ASSERT(x, ...) { if(!(x)) { AST_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
    #define AST_CORE_ASSERT(x, ...) { if(!(x)) { AST_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
    #define AST_ASSERT(x, ...)
    #define AST_CORE_ASSERT(x, ...)
#endif
