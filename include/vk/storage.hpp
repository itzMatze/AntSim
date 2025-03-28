#pragma once

#include <vector>
#include <unordered_map>
#include "vk/buffer.hpp"
#include "vk/image.hpp"

namespace ve
{
class Storage
{
public:
  Storage(const VulkanMainContext& vmc, VulkanCommandContext& vcc);

  template<typename... Args>
  uint32_t add_buffer(const std::string& name, Args&&... args)
  {
    if (buffer_names.contains(name))
    {
      if (buffers.at(buffer_names.at(name)).has_value())
      {
        // buffer name is already taken by an existing buffer
				antlog::warn("Duplicate buffer name: {}", name);
      }
      else
      {
        // buffer name exists but the corresponding buffer got deleted; so, reuse the name
        buffers.at(buffer_names.at(name)).emplace(vmc, vcc, std::forward<Args>(args)...);
      }
    }
    else
    {
      buffers.emplace_back(std::make_optional<Buffer>(vmc, vcc, std::forward<Args>(args)...));
      buffer_names.emplace(name, uint32_t(buffers.size() - 1));
    }
    const vk::Buffer& b = buffers.at(buffer_names.at(name)).value().get();
    vk::DebugUtilsObjectNameInfoEXT dmoni(b.objectType, uint64_t(static_cast<vk::Buffer::CType>(b)), name.c_str());
    vmc.logical_device.get().setDebugUtilsObjectNameEXT(dmoni);
    return buffer_names.at(name);
  }

  template<typename... Args>
  uint32_t add_image(const std::string& name, Args&&... args)
  {
    if (image_names.contains(name))
    {
      if (images.at(image_names.at(name)).has_value())
      {
        // image name is already taken by an existing image
				antlog::warn("Duplicate image name: {}", name);
      }
      else
      {
        // image name exists but the corresponding image got deleted; so, reuse the name
        images.at(image_names.at(name)).emplace(vmc, vcc, std::forward<Args>(args)...);
      }
    }
    else
    {
      images.emplace_back(std::make_optional<Image>(vmc, vcc, std::forward<Args>(args)...));
      image_names.emplace(name, images.size() - 1);
    }
    const vk::Image& i = images.at(image_names.at(name)).value().get_image();
    vk::DebugUtilsObjectNameInfoEXT dmoni(i.objectType, uint64_t(static_cast<vk::Image::CType>(i)), name.c_str());
    vmc.logical_device.get().setDebugUtilsObjectNameEXT(dmoni);
    return image_names.at(name);
  }

  void destroy_buffer(uint32_t idx);
  void destroy_image(uint32_t idx);
  void destroy_buffer(const std::string& name);
  void destroy_image(const std::string& name);
  void clear();
  Buffer& get_buffer(uint32_t idx);
  Image& get_image(uint32_t idx);
  Buffer& get_buffer_by_name(const std::string& name);
  Image& get_image_by_name(const std::string& name);

private:
  const VulkanMainContext& vmc;
  VulkanCommandContext& vcc;
  std::vector<std::optional<Buffer>> buffers;
  std::vector<std::optional<Image>> images;
  std::unordered_map<std::string, uint32_t> buffer_names;
  std::unordered_map<std::string, uint32_t> image_names;
};
} // namespace ve
