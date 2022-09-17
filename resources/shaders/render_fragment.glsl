#version 330 core

out vec4 frag_color;

in vec2 uv;

uniform sampler2D render_texture;
uniform sampler2D overlay_texture;
uniform float overlay_alpha;
uniform bool use_perspective;

void main() {
	float intensity = 1.4f;
	vec2 dist = vec2(distance(uv.x, 0.5f), distance(uv.y, 0.5f));

	dist.x *= dist.x;
	if(uv.y > 0.5f)
		dist.x *= -1;

	dist.x *= dist.y;

	vec2 coords = uv + vec2(0.0f, dist.x * intensity);

	/* TODO: Probably optimize this */
	if(use_perspective) {
		frag_color = texture(render_texture, coords);
	} else {
		frag_color = texture(render_texture, uv);
	}

	frag_color += (texture(overlay_texture, uv) * vec4(overlay_alpha));
}
