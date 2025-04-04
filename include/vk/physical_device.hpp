#pragma once

#include <optional>
#include <vulkan/vulkan.hpp>

#include "vk/extensions_handler.hpp"
#include "vk/instance.hpp"

namespace ve
{
struct QueueFamilyIndices {
	QueueFamilyIndices() : graphics(uint32_t(-1)), compute(uint32_t(-1)), transfer(uint32_t(-1)), present(uint32_t(-1))
	{}
	QueueFamilyIndices(uint32_t value) : graphics(value), compute(value), transfer(value), present(value)
	{}
	uint32_t graphics;
	uint32_t compute;
	uint32_t transfer;
	uint32_t present;
};

class PhysicalDevice
{
public:
	PhysicalDevice() = default;
	void construct(const Instance& instance, const std::optional<vk::SurfaceKHR>& surface);
	vk::PhysicalDevice get() const;
	QueueFamilyIndices get_queue_families(const std::optional<vk::SurfaceKHR>& surface) const;
	const std::vector<const char*>& get_extensions() const;
	const std::vector<const char*>& get_missing_extensions();

private:
	vk::PhysicalDevice physical_device;
	ExtensionsHandler extensions_handler;

	bool is_device_suitable(uint32_t idx, const vk::PhysicalDevice p_device, const std::optional<vk::SurfaceKHR>& surface);
	bool is_swapchain_supported(const vk::PhysicalDevice p_device, const vk::SurfaceKHR& surface) const;
	int32_t get_queue_score(vk::QueueFamilyProperties queue_family, vk::QueueFlagBits target) const;
};
} // namespace ve
