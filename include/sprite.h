#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>
#include <cglm/cglm.h>

typedef struct {
	uint32_t vao;
	uint32_t vbo;
	uint32_t texture;
	mat4 matrix;
} sprite_t;

void sprite_create(sprite_t *sprite, uint32_t texture, vec2 size);
void sprite_draw(sprite_t sprite, uint32_t shader);

#endif
