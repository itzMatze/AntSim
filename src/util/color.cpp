#include "util/color.hpp"
#include <algorithm>

uint32_t get_hex_color(const glm::vec3& value)
{
  // color values > 1.0 are allowed, but they need to be clamped when the color gets converted to hex representation
  uint32_t hex_color = std::clamp(static_cast<uint64_t>(value.r * 255.999f), uint64_t(0), uint64_t(255));
  hex_color <<= 8;
  hex_color += std::clamp(static_cast<uint64_t>(value.g * 255.999f), uint64_t(0), uint64_t(255));
  hex_color <<= 8;
  hex_color += std::clamp(static_cast<uint64_t>(value.b * 255.999f), uint64_t(0), uint64_t(255));
  hex_color <<= 8;
  return hex_color;
}

uint32_t get_hex_color(const glm::vec4& value)
{
  // color values > 1.0 are allowed, but they need to be clamped when the color gets converted to hex representation
  uint32_t hex_color = std::clamp(static_cast<uint64_t>(value.r * 255.999f), uint64_t(0), uint64_t(255));
  hex_color <<= 8;
  hex_color += std::clamp(static_cast<uint64_t>(value.g * 255.999f), uint64_t(0), uint64_t(255));
  hex_color <<= 8;
  hex_color += std::clamp(static_cast<uint64_t>(value.b * 255.999f), uint64_t(0), uint64_t(255));
  hex_color <<= 8;
  hex_color += std::clamp(static_cast<uint64_t>(value.a * 255.999f), uint64_t(0), uint64_t(255));
  return hex_color;
}
