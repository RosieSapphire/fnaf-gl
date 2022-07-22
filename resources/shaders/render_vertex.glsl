#version 330 core

layout(location = 0) in vec4 a_vertex;

out vec2 uv;

void main() {
	gl_Position = vec4(a_vertex.xy, 0.0f, 1.0f);
	uv = a_vertex.zw;
}
