#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "file.h"
#include "glyph.h"
#include "texture.h"
#include "sprite.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define WINDOW_WIDTH	1280
#define WINDOW_HEIGHT	720

int main() {
	ivec2 monitor_size;
	GLFWwindow *window;

	double time_now, time_last;

	uint8_t render_wireframe = 0;
	uint8_t render_wireframe_pressed = 0;

	sprite_t office_sprite;
	uint32_t office_texture;
	sprite_t test_sprite;
	uint32_t test_texture;

	uint32_t sprite_shader_program;

	uint32_t font_vao;
	uint32_t font_vbo;
	uint32_t font_shader_program;

	float office_look_current = 0.0f;

	mat4 matrix_projection;
	mat4 matrix_view;
	mat4 matrix_office;

	/* load GLFW */
	if(!glfwInit()) {
		printf("ERROR: GLFW fucked up.\n");
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

	/* create window */
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Five Nights at Freddy's", NULL, NULL);
	if(!window) {
		printf("ERROR: Window fucked up.\n");
		return 1;
	}

	{ /* get monitor properties */
		GLFWmonitor *monitor;
		int32_t monitor_count;
		monitor = *glfwGetMonitors(&monitor_count);
		if(!monitor) {
			printf("ERROR: Monitor fucked up.\n");
			return 1;
		}

		glfwGetMonitorWorkarea(monitor, NULL, NULL, &monitor_size[0], &monitor_size[1]);
		glfwSetWindowPos(window, (monitor_size[0] / 2) - (WINDOW_WIDTH / 2), (monitor_size[1] / 2) - (WINDOW_HEIGHT / 2));
	}

	glfwMakeContextCurrent(window);

	/* load GLAD */
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("ERROR: GLAD fucked up.\n");
		return 1;
	}
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	/* set up matricies */
	glm_ortho(0.0f, WINDOW_WIDTH, 0.0f, WINDOW_HEIGHT, -1.0f, 1.0f, matrix_projection);
	glm_mat4_copy(GLM_MAT4_IDENTITY, matrix_view);
	glm_mat4_copy(GLM_MAT4_IDENTITY, matrix_office);

	{ /* load office shaders */
		char *office_shader_vertex_source;
		char *office_shader_fragment_source;
		uint32_t office_shader_vertex;
		uint32_t office_shader_fragment;

		office_shader_vertex_source = file_load_contents("resources/shaders/sprite_vertex.glsl");
		if(!office_shader_vertex_source) {
			printf("ERROR: Vertex shader loading fucked up.\n");
			return 1;
		}

		office_shader_fragment_source = file_load_contents("resources/shaders/sprite_fragment.glsl");
		if(!office_shader_fragment_source) {
			printf("ERROR: Fragment shader loading fucked up.\n");
			return 1;
		}

		office_shader_vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(office_shader_vertex, 1, (const char *const *)(&office_shader_vertex_source), NULL);
		glCompileShader(office_shader_vertex);

		{ /* check vertex shader errors */
			int32_t success;
			char info_log[512];
			glGetShaderiv(office_shader_vertex, GL_COMPILE_STATUS, &success);
			if(!success) {
				glGetShaderInfoLog(office_shader_vertex, 512, NULL, info_log);
				printf("ERROR: Vertex shader fucked up: %s\n", info_log);
				return 1;
			}
		}

		office_shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(office_shader_fragment, 1, (const char *const *)(&office_shader_fragment_source), NULL);
		glCompileShader(office_shader_fragment);

		{ /* check vertex shader errors */
			int32_t success;
			char info_log[512];
			glGetShaderiv(office_shader_fragment, GL_COMPILE_STATUS, &success);
			if(!success) {
				glGetShaderInfoLog(office_shader_fragment, 512, NULL, info_log);
				printf("ERROR: Fragment shader fucked up: %s\n", info_log);
				return 1;
			}
		}

		free(office_shader_fragment_source);
		free(office_shader_vertex_source);

		/* link office program */
		sprite_shader_program = glCreateProgram();
		glAttachShader(sprite_shader_program, office_shader_vertex);
		glAttachShader(sprite_shader_program, office_shader_fragment);
		glLinkProgram(sprite_shader_program);

		{ /* check shader program errors */
			int32_t success;
			char info_log[512];
			glGetProgramiv(sprite_shader_program, GL_LINK_STATUS, &success);
			if(!success) {
				glGetProgramInfoLog(sprite_shader_program, 512, NULL, info_log);
				printf("ERROR: Shader program fucked up: %s\n", info_log);
				return 1;
			}
		}
		glDeleteShader(office_shader_fragment);
		glDeleteShader(office_shader_vertex);
	}

	/* set up sprites */
	office_texture = texture_create("resources/textures/office/states/normal.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	sprite_create(&office_sprite, office_texture, (vec2){1600.0f, 720.0f});

	test_texture = texture_create("resources/textures/test.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	sprite_create(&test_sprite, test_texture, (vec2){128.0f, 128.0f});

	{ /* load font shaders */
		char *font_shader_vertex_source;
		char *font_shader_fragment_source;
		uint32_t font_shader_vertex;
		uint32_t font_shader_fragment;

		font_shader_vertex_source = file_load_contents("resources/shaders/font_vertex.glsl");

		if(!font_shader_vertex_source) {
			printf("ERROR: Vertex shader loading fucked up.\n");
			return 1;
		}

		font_shader_fragment_source = file_load_contents("resources/shaders/font_fragment.glsl");
		if(!font_shader_fragment_source) {
			printf("ERROR: Fragment shader loading fucked up.\n");
			return 1;
		}

		font_shader_vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(font_shader_vertex, 1, (const char *const *)(&font_shader_vertex_source), NULL);
		glCompileShader(font_shader_vertex);

		{ /* check vertex shader errors */
			int32_t success;
			char info_log[512];
			glGetShaderiv(font_shader_vertex, GL_COMPILE_STATUS, &success);
			if(!success) {
				glGetShaderInfoLog(font_shader_vertex, 512, NULL, info_log);
				printf("ERROR: Vertex shader fucked up: %s\n", info_log);
				return 1;
			}
		}

		font_shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(font_shader_fragment, 1, (const char *const *)(&font_shader_fragment_source), NULL);
		glCompileShader(font_shader_fragment);

		{ /* check vertex shader errors */
			int32_t success;
			char info_log[512];
			glGetShaderiv(font_shader_fragment, GL_COMPILE_STATUS, &success);
			if(!success) {
				glGetShaderInfoLog(font_shader_fragment, 512, NULL, info_log);
				printf("ERROR: Fragment shader fucked up: %s\n", info_log);
				return 1;
			}
		}

		free(font_shader_fragment_source);
		free(font_shader_vertex_source);

		/* link font program */
		font_shader_program = glCreateProgram();
		glAttachShader(font_shader_program, font_shader_vertex);
		glAttachShader(font_shader_program, font_shader_fragment);
		glLinkProgram(font_shader_program);

		{ /* check shader program errors */
			int32_t success;
			char info_log[512];
			glGetProgramiv(font_shader_program, GL_LINK_STATUS, &success);
			if(!success) {
				glGetProgramInfoLog(font_shader_program, 512, NULL, info_log);
				printf("ERROR: Shader program fucked up: %s\n", info_log);
				return 1;
			}
		}
		glDeleteShader(font_shader_fragment);
		glDeleteShader(font_shader_vertex);
	}

	{ /* load freetype */
		FT_Library freetype;
		FT_Face freetype_face;
		if(FT_Init_FreeType(&freetype)) {
			printf("ERROR: Freetype fucked up.\n");
			return 1;
		}

		if(FT_New_Face(freetype, "resources/fonts/main.ttf", 0, &freetype_face)) {
			printf("ERROR: Freetype Face fucked up.\n");
			return 1;
		}

		FT_Set_Pixel_Sizes(freetype_face, 0, 48);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		for(uint8_t c = 0; c < 128; c++) {
			uint32_t glyph_texture;
			if(FT_Load_Char(freetype_face, c, FT_LOAD_RENDER)) {
				printf("ERROR: Freetype fucked up loading a char.\n");
				continue;
			}

			glGenTextures(1, &glyph_texture);
			glBindTexture(GL_TEXTURE_2D, glyph_texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, freetype_face->glyph->bitmap.width, freetype_face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, freetype_face->glyph->bitmap.buffer);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glyphs[c].texture_id = glyph_texture;
			glm_ivec2_copy((ivec2){freetype_face->glyph->bitmap.width, freetype_face->glyph->bitmap.rows}, glyphs[c].size);
			glm_ivec2_copy((ivec2){freetype_face->glyph->bitmap_left, freetype_face->glyph->bitmap_top}, glyphs[c].bearing);
			glyphs[c].advance = freetype_face->glyph->advance.x;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		FT_Done_Face(freetype_face);
		FT_Done_FreeType(freetype);
	}

	/* generate freetype buffers */
	glGenVertexArrays(1, &font_vao);
	glGenBuffers(1, &font_vbo);
	glBindVertexArray(font_vao);
	glBindBuffer(GL_ARRAY_BUFFER, font_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/* enable random shit */
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* get time ready */
	time_now = glfwGetTime();
	time_last = time_now;

	/* main loop */
	while(!glfwWindowShouldClose(window)) {
		/* calculate deltatime */
		float time_delta;
		time_now = glfwGetTime();
		time_delta = time_now - time_last;
		time_last = time_now;

		/* process input */
		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, 1);
		}

		if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			if(!render_wireframe_pressed) {
				render_wireframe = !render_wireframe;
				render_wireframe_pressed = 1;
			}
		} else {
			render_wireframe_pressed = 0;
		}

		{ /* use mouse to look around office */
			double mouse_x;
			float mouse_distance_from_center;
			glfwGetCursorPos(window, &mouse_x, NULL);

			mouse_distance_from_center = mouse_x - (WINDOW_WIDTH / 2.0f);
			if(mouse_distance_from_center < -640.0f) {
				mouse_distance_from_center = -640.0f;
			}

			if(mouse_distance_from_center > 640.0f) {
				mouse_distance_from_center = 640.0f;
			}

			if(mouse_distance_from_center < 128.0f && mouse_distance_from_center > -128.0f) {
				mouse_distance_from_center = 0.0f;
			}

			office_look_current += -mouse_distance_from_center * time_delta;
			if(office_look_current > 0.0f) {
				office_look_current = 0.0f;
			}
			if(office_look_current < -320.0f) {
				office_look_current = -320.0f;
			}
		}

		/* update */
		glm_mat4_copy(GLM_MAT4_IDENTITY, matrix_view);
		glm_translate(matrix_view, (vec3){office_look_current, 0.0f, 0.0f});

		/* draw */
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		if(render_wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		/* draw office */
		glUseProgram(sprite_shader_program);
		glUniformMatrix4fv(glGetUniformLocation(sprite_shader_program, "view"), 1, GL_FALSE, (const GLfloat *)matrix_view);
		glUniformMatrix4fv(glGetUniformLocation(sprite_shader_program, "projection"), 1, GL_FALSE, (const GLfloat *)matrix_projection);

		glUniform1i(glGetUniformLocation(sprite_shader_program, "follow_camera"), 0);
		glUniform1i(glGetUniformLocation(sprite_shader_program, "use_perspective"), 1);
		sprite_draw(office_sprite, sprite_shader_program);

		glUniform1i(glGetUniformLocation(sprite_shader_program, "follow_camera"), 1);
		glUniform1i(glGetUniformLocation(sprite_shader_program, "use_perspective"), 0);
		sprite_draw(test_sprite, sprite_shader_program);

		/*
		glUseProgram(font_shader_program);
		glUniformMatrix4fv(glGetUniformLocation(font_shader_program, "projection"), 1, GL_FALSE, (const GLfloat *)matrix_projection);
		glyph_render_string("Five Nights at Freddy's", font_shader_program, font_vao, font_vbo, 128.0f, 128.0f, 2.3f, GLM_VEC3_ONE);
		*/

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	/* destroy everything */
	texture_destroy(&test_texture);
	texture_destroy(&office_texture);

	glfwTerminate();
	return 0;
}
