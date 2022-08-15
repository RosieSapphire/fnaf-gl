#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>
#include <cglm/cglm.h>

typedef struct {
	uint32_t vao;
	uint32_t vbo;
	uint32_t texture;
	uint32_t padding;
	mat4 matrix;
	vec2 size;
	uint64_t padding2;
} sprite_t;

void sprite_create(sprite_t *sprite, uint32_t texture, vec2 size);
void sprite_set_position(sprite_t *sprite, vec2 position);
void sprite_draw(sprite_t sprite, uint32_t shader);

#endif
