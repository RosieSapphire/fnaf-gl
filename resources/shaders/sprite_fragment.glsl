#version 330 core

out vec4 frag_color;

in vec2 uv;

uniform sampler2D texture_2d;
uniform bool flip_x;
uniform float alpha;

void main() {
	if(flip_x)
		frag_color = texture2D(texture_2d, vec2(-uv.x + 1, uv.y));
	else
		frag_color = texture2D(texture_2d, uv);

	frag_color.a *= alpha;
}
