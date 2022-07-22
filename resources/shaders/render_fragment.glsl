#version 330 core

out vec4 frag_color;

in vec2 uv;

uniform sampler2D render_texture;

void main() {
	float intensity = 1.4f;
	float dist_x = distance(uv.x, 0.5f);
	float dist_y = distance(uv.y, 0.5f);

	if(uv.y < 0.5f)
		dist_x *= dist_x;
	else
		dist_x *= -dist_x;

	dist_x *= dist_y;

	vec2 coords = uv + vec2(0.0f, dist_x * intensity);

	frag_color = texture(render_texture, coords);
}
