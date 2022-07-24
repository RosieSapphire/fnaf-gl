#ifndef GLYPH_H
#define GLYPH_H

#include <cglm/cglm.h>

typedef struct {
	uint32_t texture_id;
	ivec2 size;
	ivec2 bearing;
	uint32_t advance;
} glyph_t;

static glyph_t glyphs[128];

void glyph_render_string(const char *string, const uint32_t shader_program, const uint32_t vao, const uint32_t vbo, float x, float y, const float scale, const vec3 color);

#endif
