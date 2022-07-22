#version 330 core

out vec4 frag_color;

in vec2 uv;
in vec3 frag_pos;

uniform sampler2D texture_2d;
uniform bool use_perspective;

void main() {
	float intensity = 0.2f;
	float dist_x = distance(frag_pos.x, 0.0f);
	float dist_y = distance(frag_pos.y, 0.0f);

	if(frag_pos.y < 0.0f)
		dist_x *= dist_x;
	else
		dist_x *= -dist_x;

	dist_x *= dist_y;

	vec2 coords = uv + vec2(0.0f, dist_x * intensity);

	if(use_perspective)
		frag_color = texture2D(texture_2d, coords);
	else
		frag_color = texture2D(texture_2d, uv);
}
