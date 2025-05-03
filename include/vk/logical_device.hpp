#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "vk/queue_families.hpp"
#include "vk/physical_device.hpp"

namespace ve
{
enum class QueueIndex
{
	Graphics,
	Compute,
	Transfer,
	Present
};

class LogicalDevice
{
public:
	LogicalDevice() = default;
	void construct(const PhysicalDevice& p_device, const QueueFamilies& queue_families, std::unordered_map<QueueIndex, vk::Queue>& queues);
	void destruct();
	const vk::Device& get() const;

private:
	vk::Device device;
};
} // namespace ve
