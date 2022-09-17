#ifndef FONT_H
#define FONT_H

#include <stdint.h>

typedef struct {
	uint32_t texture;
	int32_t size[2];
	int32_t bearing[2];
	uint32_t advance;
} character_t;

typedef struct {
	uint32_t vao;
	uint32_t vbo;
	character_t *characters;
} font_t;

void font_shader_create(void);
font_t font_create(const char *path);
void font_draw(const font_t font, const char *string, float *pos, const float *color, const float scale);
void font_destroy(font_t *font);
void font_shader_destroy(void);

#endif
