#version 330 core

layout(location = 0) in vec4 a_vertex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool follow_camera;

out vec2 uv;

void main() {
	if(follow_camera)
		gl_Position = projection * model * vec4(a_vertex.xy, 0.0f, 1.0f);
	else
		gl_Position = projection * view * model * vec4(a_vertex.xy, 0.0f, 1.0f);

	uv = a_vertex.zw;
}
