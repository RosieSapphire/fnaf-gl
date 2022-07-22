#version 330 core

out vec4 frag_color;

in vec2 uv;

uniform sampler2D texture_2d;

void main() {
	frag_color = texture(texture_2d, uv);
}
