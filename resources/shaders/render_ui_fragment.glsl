#version 330 core

out vec4 frag_color;

in vec2 uv;

uniform sampler2D texture_2d;
uniform float alpha;

void main() {
	frag_color = texture2D(texture_2d, uv);
	frag_color.a *= alpha;
}
