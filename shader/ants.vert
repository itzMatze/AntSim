#version 460

layout(constant_id = 0) const uint POINT_SIZE = 1;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec3 frag_color;

void main() {
	gl_PointSize = float(POINT_SIZE);
	gl_Position = vec4(pos, 0.0, 1.0);
	frag_color = vec3(1.0, 0.0, 1.0);
}
