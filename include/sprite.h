#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>
#include <cglm/cglm.h>
#include "texture.h"

typedef struct {
	uint32_t vao;
	uint32_t vbo;
	uint64_t padding0;
	vec2 position;
	vec2 size;
	texture_t *textures;
	uint16_t texture_count;
	uint16_t padding3;
	uint32_t padding2;
	uint64_t padding1;
} sprite_t;

sprite_t sprite_create(vec2 pos, vec2 size, const char *path_format, const uint16_t texture_count);
void sprite_draw(sprite_t sprite, uint32_t shader, const uint16_t texture_index);
void sprite_destroy(sprite_t *sprite);

#endif
