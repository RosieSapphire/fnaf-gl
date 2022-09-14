#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>
#include <cglm/cglm.h>
#include "texture.h"

typedef struct {
	uint32_t vao;
	uint32_t vbo;
	uint64_t padding0;
	mat4 matrix;
	vec2 size;
	texture_t *textures;
	uint16_t texture_count;
	uint16_t padding3;
	uint32_t padding2;
	uint64_t padding1;
} sprite_t;

void sprite_create(sprite_t *sprite, vec2 pos, vec2 size, const char *path_format, const uint16_t texture_count);
void sprite_set_position(sprite_t *sprite, vec2 position);
void sprite_draw(sprite_t sprite, uint32_t shader, const uint16_t texture_index);
void sprite_destroy(sprite_t *sprite);

#endif
