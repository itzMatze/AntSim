#pragma once

#include <cstdint>
#include "vec3.hpp"
#include "vec4.hpp"

uint32_t get_hex_color(const glm::vec3& value);
uint32_t get_hex_color(const glm::vec4& value);
