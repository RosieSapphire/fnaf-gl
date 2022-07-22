#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "file.h"
#include "glyph.h"
#include "texture.h"
#include "sprite.h"
#include "shader.h"
#include "mouse.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define WINDOW_WIDTH					1280
#define WINDOW_HEIGHT					720

#define ANIMATION_FRAMETIME				0.01666667f

#define DOOR_BUTTON_LEFT_DOOR_FLAG		0x1
#define DOOR_BUTTON_LEFT_LIGHT_FLAG		0x2
#define DOOR_BUTTON_RIGHT_DOOR_FLAG		0x4
#define DOOR_BUTTON_RIGHT_LIGHT_FLAG	0x8

#define DOOR_BUTTON_LEFT_00_INDEX		0
#define DOOR_BUTTON_LEFT_01_INDEX		1
#define DOOR_BUTTON_LEFT_10_INDEX		2
#define DOOR_BUTTON_LEFT_11_INDEX		3
#define DOOR_BUTTON_RIGHT_00_INDEX		4
#define DOOR_BUTTON_RIGHT_01_INDEX		5
#define DOOR_BUTTON_RIGHT_10_INDEX		6
#define DOOR_BUTTON_RIGHT_11_INDEX		7

int main() {
	ivec2 monitor_size;
	GLFWwindow *window;

	ivec2 mouse_position;
	uint8_t mouse_has_clicked = 0;

	uint32_t fbo;
	uint32_t rbo;
	uint32_t render_vao;
	uint32_t render_vbo;

	double time_now, time_last;
	float animation_timer = 0.0f;

	uint8_t render_wireframe = 0;
	uint8_t render_wireframe_pressed = 0;
	uint32_t render_texture;
	uint32_t render_shader_program;

	sprite_t office_sprite;
	uint32_t office_texture;

	sprite_t fan_animation_sprite;
	uint32_t fan_animation_textures[3];
	uint32_t fan_animation_frame = 0;

	sprite_t door_button_left_sprite;
	sprite_t door_button_right_sprite;
	uint32_t door_button_textures[8];
	uint8_t door_button_flags = 0;

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

	/* create shaders */
	render_shader_program = shader_create("resources/shaders/render_vertex.glsl", "resources/shaders/render_fragment.glsl");
	font_shader_program = shader_create("resources/shaders/font_vertex.glsl", "resources/shaders/font_fragment.glsl");
	sprite_shader_program = shader_create("resources/shaders/sprite_vertex.glsl", "resources/shaders/sprite_fragment.glsl");

	/* set up sprites */
	office_texture = texture_create("resources/textures/office/states/normal.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	sprite_create(&office_sprite, office_texture, (vec2){1600.0f, 720.0f});

	fan_animation_textures[0] = texture_create("resources/textures/office/fan/00.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	fan_animation_textures[1] = texture_create("resources/textures/office/fan/01.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	fan_animation_textures[2] = texture_create("resources/textures/office/fan/02.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	sprite_create(&fan_animation_sprite, fan_animation_textures[0], (vec2){137.0f, 196.0f});
	sprite_set_position(&fan_animation_sprite, (vec3){780.0f, 303.0f});

	door_button_textures[0] = texture_create("resources/textures/office/doors/buttons/left-00.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	door_button_textures[1] = texture_create("resources/textures/office/doors/buttons/left-01.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	door_button_textures[2] = texture_create("resources/textures/office/doors/buttons/left-10.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	door_button_textures[3] = texture_create("resources/textures/office/doors/buttons/left-11.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	sprite_create(&door_button_left_sprite, door_button_textures[0], (vec2){92.0f, 247.0f});
	sprite_set_position(&door_button_left_sprite, (vec2){6.0f, 263.0f});

	door_button_textures[4] = texture_create("resources/textures/office/doors/buttons/right-00.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	door_button_textures[5] = texture_create("resources/textures/office/doors/buttons/right-01.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	door_button_textures[6] = texture_create("resources/textures/office/doors/buttons/right-10.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	door_button_textures[7] = texture_create("resources/textures/office/doors/buttons/right-11.png", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	sprite_create(&door_button_right_sprite, door_button_textures[4], (vec2){92.0f, 247.0f});
	sprite_set_position(&door_button_right_sprite, (vec2){1497.0f, 273.0f});

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

	/* set up framebuffer */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &render_texture);
	glBindTexture(GL_TEXTURE_2D, render_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_texture, 0);

	/* set up renderbuffer */
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("ERROR: Framebuffer fucked up.\n");
		return 1;
	}

	{/* generate buffers for render texture */
		float render_vertices[] = {
			-1.0f,	-1.0f,	0.0f, 0.0f,
			 1.0f,	-1.0f,	1.0f, 0.0f,
			 1.0f,	 1.0f,	1.0f, 1.0f,

			-1.0f,	-1.0f,	0.0f, 0.0f,
			 1.0f,	 1.0f,	1.0f, 1.0f,
			-1.0f,	 1.0f,	0.0f, 1.0f,
		};

		glGenVertexArrays(1, &render_vao);
		glBindVertexArray(render_vao);

		glGenBuffers(1, &render_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, render_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, render_vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

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

		mouse_get_position(window, mouse_position);
		{ /* use mouse to look around office */
			float mouse_distance_from_center;
			mouse_distance_from_center = mouse_position[0] - (WINDOW_WIDTH / 2.0f);

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

		/* check for clicking door buttons */
		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !mouse_has_clicked) {
			int32_t mouse_offset = (int32_t)office_look_current;
			uint8_t door_button_flags_old = door_button_flags;
			door_button_flags ^= DOOR_BUTTON_LEFT_DOOR_FLAG *	mouse_inside_box(window, (ivec4){27		+ mouse_offset, 89		+ mouse_offset, 251, 371});
			door_button_flags ^= DOOR_BUTTON_LEFT_LIGHT_FLAG *	mouse_inside_box(window, (ivec4){25 	+ mouse_offset, 87 	 	+ mouse_offset, 393, 513});
			door_button_flags ^= DOOR_BUTTON_RIGHT_DOOR_FLAG * 	mouse_inside_box(window, (ivec4){1519	+ mouse_offset, 1581 	+ mouse_offset, 267, 387});
			door_button_flags ^= DOOR_BUTTON_RIGHT_LIGHT_FLAG * mouse_inside_box(window, (ivec4){1519	+ mouse_offset, 1581	+ mouse_offset, 398, 518});

			/* handle cases where both lights are toggled */
			if((door_button_flags & DOOR_BUTTON_LEFT_LIGHT_FLAG) && (door_button_flags_old & DOOR_BUTTON_RIGHT_LIGHT_FLAG)) {
				door_button_flags &= ~DOOR_BUTTON_RIGHT_LIGHT_FLAG;
			}

			if((door_button_flags & DOOR_BUTTON_RIGHT_LIGHT_FLAG) && (door_button_flags_old & DOOR_BUTTON_LEFT_LIGHT_FLAG)) {
				door_button_flags &= ~DOOR_BUTTON_LEFT_LIGHT_FLAG;
			}

			/* update button textures */
			if(door_button_flags & DOOR_BUTTON_LEFT_DOOR_FLAG) {
				if(door_button_flags & DOOR_BUTTON_LEFT_LIGHT_FLAG) {
					door_button_left_sprite.texture = door_button_textures[DOOR_BUTTON_LEFT_11_INDEX];
				} else {
					door_button_left_sprite.texture = door_button_textures[DOOR_BUTTON_LEFT_10_INDEX];
				}
			} else {
				if(door_button_flags & DOOR_BUTTON_LEFT_LIGHT_FLAG) {
					door_button_left_sprite.texture = door_button_textures[DOOR_BUTTON_LEFT_01_INDEX];
				} else {
					door_button_left_sprite.texture = door_button_textures[DOOR_BUTTON_LEFT_00_INDEX];
				}
			}

			if(door_button_flags & DOOR_BUTTON_RIGHT_DOOR_FLAG) {
				if(door_button_flags & DOOR_BUTTON_RIGHT_LIGHT_FLAG) {
					door_button_right_sprite.texture = door_button_textures[DOOR_BUTTON_RIGHT_11_INDEX];
				} else {
					door_button_right_sprite.texture = door_button_textures[DOOR_BUTTON_RIGHT_10_INDEX];
				}
			} else {
				if(door_button_flags & DOOR_BUTTON_RIGHT_LIGHT_FLAG) {
					door_button_right_sprite.texture = door_button_textures[DOOR_BUTTON_RIGHT_01_INDEX];
				} else {
					door_button_right_sprite.texture = door_button_textures[DOOR_BUTTON_RIGHT_00_INDEX];
				}
			}

			mouse_has_clicked = 1;
		}

		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE) {
			mouse_has_clicked = 0;
		}

		/* update */
		printf("Door Flags: %d", (door_button_flags & DOOR_BUTTON_RIGHT_LIGHT_FLAG) > 0);
		printf("%d", (door_button_flags & DOOR_BUTTON_RIGHT_DOOR_FLAG) > 0);
		printf("%d", (door_button_flags & DOOR_BUTTON_LEFT_LIGHT_FLAG) > 0);
		printf("%d\n", (door_button_flags & DOOR_BUTTON_LEFT_DOOR_FLAG) > 0);

		animation_timer += time_delta;
		if(animation_timer > ANIMATION_FRAMETIME) {
			animation_timer = 0.0f;
			fan_animation_frame++;
			fan_animation_frame %= 3;
			fan_animation_sprite.texture = fan_animation_textures[fan_animation_frame];
		}

		glm_mat4_copy(GLM_MAT4_IDENTITY, matrix_view);
		glm_translate(matrix_view, (vec3){office_look_current, 0.0f, 0.0f});

		/* draw */
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
		sprite_draw(office_sprite, sprite_shader_program);
		sprite_draw(fan_animation_sprite, sprite_shader_program);
		sprite_draw(door_button_left_sprite, sprite_shader_program);
		sprite_draw(door_button_right_sprite, sprite_shader_program);

		/*
		glUseProgram(font_shader_program);
		glUniformMatrix4fv(glGetUniformLocation(font_shader_program, "projection"), 1, GL_FALSE, (const GLfloat *)matrix_projection);
		glyph_render_string("Five Nights at Freddy's", font_shader_program, font_vao, font_vbo, 128.0f, 128.0f, 2.3f, GLM_VEC3_ONE);
		*/

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(render_shader_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, render_texture);
		glUniform1i(glGetUniformLocation(render_shader_program, "render_texture"), 0);
		glBindVertexArray(render_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	/* destroy everything */
	glDeleteFramebuffers(1, &fbo);
	texture_destroy(&fan_animation_textures[0]);
	texture_destroy(&fan_animation_textures[1]);
	texture_destroy(&fan_animation_textures[2]);
	texture_destroy(&office_texture);
	glDeleteShader(sprite_shader_program);
	glDeleteShader(font_shader_program);
	glDeleteShader(render_shader_program);

	glfwTerminate();
	return 0;
}
