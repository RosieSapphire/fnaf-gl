#version 330 core

layout(location = 0) in vec4 a_vertex;

uniform mat4 projection;

out vec2 uv;

void main() {
	gl_Position = projection * vec4(a_vertex.xy, 0.0, 1.0);
	uv = a_vertex.zw;
}
