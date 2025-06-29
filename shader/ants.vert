#version 460

#extension GL_GOOGLE_include_directive: require
#include "uniform_buffer_struct.glsl"

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 dir;
layout(location = 2) in uint state_bits;

layout(location = 0) out vec2 frag_uv;
layout(location = 1) out flat int frag_ant_id;

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
	vec2 vert_pos = quad_offsets[gl_VertexIndex] * quad_size;
	const vec2 norm_front = normalize(quad_offsets[2]);
	const vec2 norm_dir = normalize(dir);
	float cos_a = dot(norm_front, norm_dir);
	float sin_a = norm_front.x * norm_dir.y - norm_front.y * norm_dir.x;
	const mat2 rotation = mat2(
		cos_a, sin_a,
		-sin_a,  cos_a
	);
	vert_pos = rotation * vert_pos + pos;
	vec2 ndc = (2.0 * (vert_pos - ub.range_min)) / (ub.range_max - ub.range_min) - 1.0;
	gl_Position = vec4(ndc, 0.0, 1.0);
	if (gl_VertexIndex == 0 || gl_VertexIndex == 3) frag_uv = vec2(0.0, 0.0);
	else if (gl_VertexIndex == 1) frag_uv = vec2(1.0, 0.0);
	else if (gl_VertexIndex == 2 || gl_VertexIndex == 4) frag_uv = vec2(1.0, 1.0);
	else if (gl_VertexIndex == 5) frag_uv = vec2(0.0, 1.0);
	frag_ant_id = gl_InstanceIndex;
}
