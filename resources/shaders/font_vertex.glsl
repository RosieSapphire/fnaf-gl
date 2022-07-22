#version 330 core

layout (location = 0) in vec4 vertex;

out vec2 uv;

uniform mat4 projection;

void main() {
	gl_Position = projection * vec4(vertex.xy, 0.0f, 1.0f);
	// gl_Position.z = 0.0f;
    uv = vertex.zw;
}
