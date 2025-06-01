#version 460

#extension GL_GOOGLE_include_directive: require
#include "ants_struct.glsl"

layout(constant_id = 0) const uint POINT_SIZE = 1;

layout(location = 0) in vec2 pos;
layout(location = 1) in uint state_bits;

layout(location = 0) out vec3 frag_color;

struct PushConstants
{
	vec2 range_min;
	vec2 range_max;
};

layout(push_constant) uniform PushConstant { PushConstants pc; };

void main() {
	gl_PointSize = float(POINT_SIZE);
	vec2 ndc = (2.0 * (pos - pc.range_min)) / (pc.range_max - pc.range_min) - 1.0;
	gl_Position = vec4(ndc, 0.0, 1.0);
	if ((state_bits & ANT_HAS_FOOD) == 0) frag_color = vec3(1.0, 0.0, 1.0);
	else frag_color = vec3(1.0, 0.0, 0.0);
}
