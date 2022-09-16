#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include "file.h"

shader_t shader_create(const char *shader_vertex_path, const char *shader_fragment_path) {
	const uint32_t shader_types[2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
	const char *shader_paths[2] = {shader_vertex_path, shader_fragment_path};
	#ifdef DEBUG
		const char *shader_type_names[2] = {"Vertex", "Fragment"};
	#endif
	char *shader_sources[2];
	shader_t shaders[2];
	shader_t shader_program;

	shader_program = glCreateProgram();
	for(uint8_t i = 0; i < 2; i++) {
		#ifdef DEBUG
			int32_t success;
			char info_log[512];
		#endif
		shader_sources[i] = file_load_contents(shader_paths[i]);
		#ifdef DEBUG
			if(!shader_sources[i]) {
				printf("ERROR: %s shader loading fucked up.\n", shader_type_names[i]);
				return 0;
			}
		#endif

		shaders[i] = glCreateShader(shader_types[i]);
		glShaderSource(shaders[i], 1, (const char *const *)(&shader_sources[i]), NULL);
		glCompileShader(shaders[i]);

		#ifdef DEBUG
			glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &success);
			if(!success) {
				glGetShaderInfoLog(shaders[i], 512, NULL, info_log);
				printf("ERROR: Vertex shader fucked up: %s\n", info_log);
				return 0;
			}
		#endif

		glAttachShader(shader_program, shaders[i]);
		glDeleteShader(shaders[i]);
		free(shader_sources[i]);
	}

	glLinkProgram(shader_program);

	return shader_program;
}
