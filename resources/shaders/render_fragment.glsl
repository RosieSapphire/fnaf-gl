#version 330 core

out vec4 frag_color;

in vec2 uv;

uniform sampler2D render_texture;

void main() {
	float intensity = 1.4f;
	vec2 dist = vec2(distance(uv.x, 0.5f), distance(uv.y, 0.5f));

	dist.x *= dist.x;
	if(uv.y > 0.5f)
		dist.x *= -1;

	dist.x *= dist.y;

	vec2 coords = uv + vec2(0.0f, dist.x * intensity);

	frag_color = texture(render_texture, coords);
}
