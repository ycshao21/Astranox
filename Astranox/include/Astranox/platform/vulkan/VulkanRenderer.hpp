#pragma once

#include "VulkanPipeline.hpp"

namespace Astranox
{
	class VulkanRenderer: public RefCounted
	{
	public:
		VulkanRenderer();
		virtual ~VulkanRenderer() = default;

		void init();
		void shutdown();
	};
}
