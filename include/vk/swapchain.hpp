#pragma once

#include "vk/common.hpp"
#include "vk/render_pass.hpp"
#include "vk/storage.hpp"
#include "vk/vulkan_main_context.hpp"

namespace ve
{
class Swapchain
{
public:
	Swapchain(const VulkanMainContext& vmc, VulkanCommandContext& vcc, Storage& storage);
	void construct(bool vsync);
	void destruct();
	void recreate(bool vsync);
	const vk::SwapchainKHR& get() const;
	const RenderPass& get_render_pass() const;
	vk::Extent2D get_extent() const;
	vk::Framebuffer get_framebuffer(uint32_t idx) const;

private:
	const VulkanMainContext& vmc;
	VulkanCommandContext& vcc;
	Storage& storage;
	vk::Extent2D extent;
	vk::SurfaceFormatKHR surface_format;
	vk::Format depth_format;
	vk::SwapchainKHR swapchain;
	RenderPass render_pass;
	uint32_t depth_buffer;
	std::vector<vk::Image> images;
	std::vector<vk::ImageView> image_views;
	std::vector<vk::Framebuffer> framebuffers;

	vk::SwapchainKHR create_swapchain(bool vsync);
	void create_framebuffers();
	vk::PresentModeKHR choose_present_mode(bool vsync);
	vk::Extent2D choose_extent();
	vk::SurfaceFormatKHR choose_surface_format();
	vk::Format choose_depth_format();
	void construct(bool vsync, bool full);
	void destruct(bool full);
};
} // namespace ve
