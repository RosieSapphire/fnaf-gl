#include "shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include "file.h"

uint32_t shader_create(const char *shader_vertex_path, const char *shader_fragment_path) {
	char *shader_vertex_source;
	char *shader_fragment_source;
	uint32_t shader_vertex;
	uint32_t shader_fragment;

	uint32_t shader_program;

	shader_vertex_source = file_load_contents(shader_vertex_path);
	if(!shader_vertex_source) {
		printf("ERROR: Vertex shader loading fucked up.\n");
		return 0;
	}

	shader_fragment_source = file_load_contents(shader_fragment_path);
	if(!shader_fragment_source) {
		printf("ERROR: Fragment shader loading fucked up.\n");
		return 0;
	}

	shader_vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader_vertex, 1, (const char *const *)(&shader_vertex_source), NULL);
	glCompileShader(shader_vertex);

	{ /* check vertex shader errors */
		int32_t success;
		char info_log[512];
		glGetShaderiv(shader_vertex, GL_COMPILE_STATUS, &success);
		if(!success) {
			glGetShaderInfoLog(shader_vertex, 512, NULL, info_log);
			printf("ERROR: Vertex shader fucked up: %s\n", info_log);
			return 1;
		}
	}

	shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader_fragment, 1, (const char *const *)(&shader_fragment_source), NULL);
	glCompileShader(shader_fragment);

	{ /* check vertex shader errors */
		int32_t success;
		char info_log[512];
		glGetShaderiv(shader_fragment, GL_COMPILE_STATUS, &success);
		if(!success) {
			glGetShaderInfoLog(shader_fragment, 512, NULL, info_log);
			printf("ERROR: Fragment shader fucked up: %s\n", info_log);
			return 1;
		}
	}

	free(shader_fragment_source);
	free(shader_vertex_source);

	/* link office program */
	shader_program = glCreateProgram();
	glAttachShader(shader_program, shader_vertex);
	glAttachShader(shader_program, shader_fragment);
	glLinkProgram(shader_program);

	{ /* check shader program errors */
		int32_t success;
		char info_log[512];
		glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
		if(!success) {
			glGetProgramInfoLog(shader_program, 512, NULL, info_log);
			printf("ERROR: Shader program fucked up: %s\n", info_log);
			return 1;
		}
	}
	glDeleteShader(shader_fragment);
	glDeleteShader(shader_vertex);

	return shader_program;
}
