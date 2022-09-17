#version 330 core

in vec2 uv;

uniform sampler2D text;
uniform vec3 text_color;

out vec4 frag_color;

void main() {
	frag_color = vec4(text_color.rgb, texture(text, uv).r);
}
