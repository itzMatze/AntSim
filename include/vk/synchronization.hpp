#pragma once

#include <vulkan/vulkan.hpp>

namespace ve
{
class Synchronization
{
public:
	enum SemaphoreNames
	{
		S_IMAGE_AVAILABLE = 0,
		S_RENDER_FINISHED = 1,
		S_ANTS_STEP_FINISHED = 2,
		S_FRAME_TO_FRAME = 3,
		SEMAPHORE_COUNT
	};

	enum FenceNames
	{
		F_RENDER_FINISHED = 0,
		F_COMPUTE_FINISHED = 1,
		FENCE_COUNT
	};

	Synchronization(const vk::Device& logical_device);
	void construct();
	void destruct();
	const vk::Semaphore& get_semaphore(SemaphoreNames name) const;
	const vk::Fence& get_fence(FenceNames name) const;
	bool is_fence_finished(FenceNames name) const;
	void wait_for_fence(FenceNames name) const;
	void reset_fence(FenceNames name) const;

private:
	const vk::Device& device;
	std::vector<vk::Semaphore> semaphores;
	std::vector<vk::Fence> fences;
};
} // namespace ve
