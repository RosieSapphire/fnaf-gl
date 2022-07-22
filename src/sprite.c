#include "sprite.h"

#include <glad/glad.h>

void sprite_create(sprite_t *sprite, uint32_t texture, vec2 size) {
	float vertices[] = {
		0.0f,		0.0f,		0.0f, 0.0f,
		size[0],	0.0f,		1.0f, 0.0f,
		size[0],	size[1],	1.0f, 1.0f,

		0.0f,		0.0f,		0.0f, 0.0f,
		size[0],	size[1],	1.0f, 1.0f,
		0.0f,		size[1], 	0.0f, 1.0f,
	};
	glm_vec2_copy(size, sprite->size);

	glGenVertexArrays(1, &sprite->vao);
	glBindVertexArray(sprite->vao);

	glGenBuffers(1, &sprite->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, sprite->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	sprite->texture = texture;
	glm_mat4_copy(GLM_MAT4_IDENTITY, sprite->matrix);
}

void sprite_set_position(sprite_t *sprite, vec2 position) {
	vec3 position_converted;
	position_converted[0] = position[0];
	position_converted[1] = 720 - position[1] - sprite->size[1];
	position_converted[2] = 0.0f;

	glm_mat4_copy(sprite->matrix, GLM_MAT4_IDENTITY);
	glm_translate(sprite->matrix, position_converted);
}

void sprite_draw(sprite_t sprite, uint32_t shader) {
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, (const GLfloat *)sprite.matrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sprite.texture);
		glBindVertexArray(sprite.vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
}
