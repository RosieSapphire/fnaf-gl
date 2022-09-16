#include "sprite.h"
#include "texture.h"

#include <glad/glad.h>
#include <stdarg.h>
#include <assert.h>

sprite_t sprite_create(vec2 pos, vec2 size, const char *path_format, const uint16_t texture_count) {
	sprite_t sprite;
	char *paths;
	uint8_t texture_path_length = 0;
	uint8_t chars_excluded = 0;
	const char *c = path_format;
	const float vertices[] = {
		0.0f,	0.0f,	0.0f, 0.0f,
		1.0f,	0.0f,	1.0f, 0.0f,
		1.0f,	1.0f,	1.0f, 1.0f,

		0.0f,	0.0f,	0.0f, 0.0f,
		1.0f,	1.0f,	1.0f, 1.0f,
		0.0f,	1.0f, 	0.0f, 1.0f,
	};

	#ifdef DEBUG
		assert(texture_count > 0);
	#endif

	glm_vec2_copy(size, sprite.size);

	glGenVertexArrays(1, &sprite.vao);
	glBindVertexArray(sprite.vao);

	glGenBuffers(1, &sprite.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, sprite.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	sprite.textures = calloc(texture_count, sizeof(texture_t));
	sprite.texture_count = texture_count;
	while(*c) {
		if(*c == '%') {
			chars_excluded++;
			continue;
		} c++;
	}

	texture_path_length = (uint8_t)(c - path_format - chars_excluded);
	if(texture_count - 1) {
		paths = calloc(texture_count * (texture_path_length + 6), sizeof(char));
		for(uint16_t i = 0; i < texture_count; i++) {
			const uint32_t offset = (i * texture_path_length);
			sprintf(paths + offset, "%s%u.png", path_format, i);
			sprite.textures[i] = texture_create(paths + offset, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
		}
	} else {
		paths = calloc(texture_count * texture_path_length, sizeof(char));
		sprintf(paths, "%s", path_format);
		*sprite.textures = texture_create(paths, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	}
	free(paths);
	glm_vec2_copy(pos, sprite.position);

	return sprite;
}

void sprite_draw(sprite_t sprite, uint32_t shader, const uint16_t texture_index) {
	mat4 sprite_matrix;
	vec3 position_converted;

	#ifdef DEBUG
		assert(texture_index < sprite.texture_count);
	#endif

	position_converted[0] = sprite.position[0];
	position_converted[1] = 720 - sprite.position[1] - sprite.size[1];
	position_converted[2] = 0.0f;

	glm_mat4_identity(sprite_matrix);
	glm_translate(sprite_matrix, position_converted);
	glm_scale(sprite_matrix, (vec3){sprite.size[0], sprite.size[1], 0.0f});
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, (const GLfloat *)sprite_matrix);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sprite.textures[texture_index]);
	glBindVertexArray(sprite.vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void sprite_destroy(sprite_t *sprite) {
	for(uint8_t i = 0; i < sprite->texture_count; i++) {
		glDeleteTextures(1, &sprite->textures[i]);
	}
	free(sprite->textures);
}
