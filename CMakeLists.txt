cmake_minimum_required(VERSION 3.19)
project(AntSim)
set(CMAKE_CXX_STANDARD 20)

set(SDL3_DIR "${PROJECT_SOURCE_DIR}/dependencies/SDL3")
set(GLM_DIR "${PROJECT_SOURCE_DIR}/dependencies/glm")

set(SOURCE_FILES
	src/ants.cpp
	src/application.cpp
	src/event_handler.cpp
	src/ui.cpp
	src/window.cpp
	src/work_context.cpp
	src/util/random_generator.cpp
	src/vk/command_pool.cpp
	src/vk/descriptor_set_handler.cpp
	src/vk/device_timer.cpp
	src/vk/extensions_handler.cpp
	src/vk/image.cpp
	src/vk/instance.cpp
	src/vk/logical_device.cpp
	src/vk/physical_device.cpp
	src/vk/pipeline.cpp
	src/vk/render_pass.cpp
	src/vk/shader.cpp
	src/vk/storage.cpp
	src/vk/swapchain.cpp
	src/vk/synchronization.cpp
	src/vk/vulkan_command_context.cpp
	src/vk/vulkan_main_context.cpp
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui.cpp"
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_sdl3.cpp"
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_vulkan.cpp"
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_draw.cpp"
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_widgets.cpp"
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/imgui_tables.cpp"
)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
find_package(spdlog REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(AntSim src/main.cpp ${SOURCE_FILES})
target_include_directories(AntSim PRIVATE "${PROJECT_SOURCE_DIR}/include"
	"${PROJECT_SOURCE_DIR}/dependencies/"
	Vulkan::Headers
	"${SDL3_DIR}/include/"
	"${GLM_DIR}/"
	"${PROJECT_SOURCE_DIR}/dependencies/VulkanMemoryAllocator/include/"
	"${PROJECT_SOURCE_DIR}/dependencies/imgui/"
)
target_link_libraries(AntSim PRIVATE spdlog::spdlog SDL3::SDL3 Vulkan::Vulkan)
