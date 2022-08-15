#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>

uint32_t texture_create(const char *path, const int32_t wrap_mode, const int32_t min_interpolation, const int32_t mag_interpolation) {
	int32_t texture_width, texture_height, texture_format;
	uint8_t *texture_data;
	uint32_t texture;
	int32_t texture_format_enums[5] = {
		0,
		GL_RED,
		GL_RG,
		GL_RGB,
		GL_RGBA
	};

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_interpolation);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_interpolation);

	stbi_set_flip_vertically_on_load(1);
	texture_data = stbi_load(path, &texture_width, &texture_height, &texture_format, 0);
	if(!texture_data) {
		printf("ERROR: Texture at: %s fucked up.\n", path);
		return 1;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, texture_format_enums[texture_format], texture_width, texture_height, 0, (uint32_t)texture_format_enums[texture_format], GL_UNSIGNED_BYTE, texture_data);
	stbi_image_free(texture_data);

	return texture;
}

void texture_destroy(uint32_t *texture) {
	glDeleteTextures(1, texture);
}
