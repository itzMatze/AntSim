#version 460

#extension GL_GOOGLE_include_directive: require
#include "uniform_buffer_struct.glsl"
#include "ants_struct.glsl"

layout(location = 0) in vec2 pos;
layout(location = 1) in uint state_bits;

layout(location = 0) out vec3 frag_color;

layout(binding = 99) uniform UniformBuffer {
	UniformBufferData ub;
};

const vec2 quad_offsets[6] = vec2[](
	vec2(-0.5, -0.5),
	vec2( 0.5, -0.5),
	vec2( 0.5,  0.5),
	vec2(-0.5, -0.5),
	vec2( 0.5,  0.5),
	vec2(-0.5,  0.5)
);

const float quad_size = 0.02;

void main() {
	const vec2 vert_pos = pos + quad_offsets[gl_VertexIndex] * quad_size;
	vec2 ndc = (2.0 * (vert_pos - ub.range_min)) / (ub.range_max - ub.range_min) - 1.0;
	gl_Position = vec4(ndc, 0.0, 1.0);
	if ((state_bits & ANT_HAS_FOOD) == 0) frag_color = vec3(1.0, 0.0, 1.0);
	else frag_color = vec3(1.0, 0.0, 0.0);
}
