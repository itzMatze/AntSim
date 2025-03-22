#version 460

#extension GL_GOOGLE_include_directive: require

layout(constant_id = 0) const uint RESOLUTION_X = 1;
layout(constant_id = 1) const uint RESOLUTION_Y = 1;

layout(location = 0) in vec3 in_color;
layout(location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(in_color, 1.0);
}
