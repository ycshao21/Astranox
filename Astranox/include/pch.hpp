#pragma once

#include <mutex>
#include <memory>
#include <concepts>
#include <utility>
#include <algorithm>
#include <functional>
#include <filesystem>

#include <string>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

#include <glm/glm.hpp>

#include <Astranox/core/Base.hpp>
#include <Astranox/core/RefCounted.hpp>
#include <Astranox/core/Logging.hpp>

#include <Astranox/core/events/Event.hpp>
#include <Astranox/core/events/ApplicationEvent.hpp>
#include <Astranox/core/events/KeyEvent.hpp>
#include <Astranox/core/events/MouseEvent.hpp>


#ifdef AST_PLATFORM_WINDOWS
    #include <Windows.h>
#endif
