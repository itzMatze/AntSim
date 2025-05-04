#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>
#include "vk/queue_families.hpp"
#include "window/window.hpp"
#include "vk/logical_device.hpp"
#include "vk/physical_device.hpp"
#include "vk_mem_alloc.h"

namespace vkte
{
class VulkanMainContext
{
public:
	VulkanMainContext() = default;
	void construct(const std::string& title, const uint32_t width, const uint32_t height);
	void destruct();
	std::vector<vk::SurfaceFormatKHR> get_surface_formats() const;
	std::vector<vk::PresentModeKHR> get_surface_present_modes() const;
	vk::SurfaceCapabilitiesKHR get_surface_capabilities() const;
	const vk::Queue& get_graphics_queue() const;
	const vk::Queue& get_transfer_queue() const;
	const vk::Queue& get_compute_queue() const;
	const vk::Queue& get_present_queue() const;

private:
	std::unordered_map<QueueIndex, vk::Queue> queues;

	void create_vma_allocator();
	void setup_debug_messenger();

public:
	vk::detail::DynamicLoader dl;
	std::unique_ptr<Window> window;
	Instance instance;
	vk::DebugUtilsMessengerEXT debug_messenger;
	vk::SurfaceKHR surface;
	PhysicalDevice physical_device;
	QueueFamilies queue_families;
	LogicalDevice logical_device;
	VmaAllocator va;
};
} // namespace vkte
