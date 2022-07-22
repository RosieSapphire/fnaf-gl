#include "glyph.h"

#include <glad/glad.h>

void glyph_render_string(const char *string, const uint32_t shader_program, const uint32_t vao, const uint32_t vbo, float x, float y, const float scale, const vec3 color) {
	glDisable(GL_DEPTH_TEST);
	glUseProgram(shader_program);
	glUniform3f(glGetUniformLocation(shader_program, "text_color"), color[0], color[1], color[2]);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vao);

	for(const char *i = string; *i != 0; i++) {
		glyph_t cur_glyph = glyphs[*((const uint8_t *)i)];
		float cur_x = x + cur_glyph.bearing[0] * scale;
		float cur_y = y - ((cur_glyph.size[1] - cur_glyph.bearing[1]) * scale);
		float w = cur_glyph.size[0] * scale;
		float h = cur_glyph.size[1] * scale;

		float font_vertices[6][4] = {
			{cur_x,		cur_y + h,	0.0f, 0.0f},
			{cur_x, 	cur_y,		0.0f, 1.0f},
			{cur_x + w, cur_y,		1.0f, 1.0f},

			{cur_x,		cur_y + h,	0.0f, 0.0f},
			{cur_x + w,	cur_y,		1.0f, 1.0f},
			{cur_x + w, cur_y + h,	1.0f, 0.0f},
		};

		glBindTexture(GL_TEXTURE_2D, glyphs[*((const uint8_t *)i)].texture_id);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(font_vertices), font_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (cur_glyph.advance >> 6) * scale;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
}
