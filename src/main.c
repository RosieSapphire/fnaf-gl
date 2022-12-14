#include <cglm/vec3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include <AL/al.h>
#include <AL/alc.h>

#include "assets.h"
#include "file.h"
#include "font.h"
#include "texture.h"
#include "sound.h"
#include "shader.h"
#include "helpers.h"

#ifdef DEBUG
	#define TIME_MULTIPLIER 			32	
#endif

#define WINDOW_WIDTH					1280
#define WINDOW_HEIGHT					720

#define ANIMATION_FRAMETIME				0.01666667f

#define DOOR_BUTTON_LIGHT_FLAG			0x1
#define DOOR_BUTTON_DOOR_FLAG			0x2
#define DOOR_BUTTON_BOTH_DOORS_FLAG 	0xA

#define CAM_TIMER_INIT 					0.35f

static GLFWwindow *window;
static uint8_t mouse_has_clicked = 0;

static float scaled_update_timer = 0.0f;

static uint32_t fbo;
static uint32_t rbo;
static uint32_t render_vao;

static double time_last;

static uint32_t render_texture;
static uint32_t render_shader_program;
static uint32_t ui_shader_program;

static assets_global_t assets_global;
static assets_title_t assets_title;
static assets_game_t assets_game;

static uint8_t blip_animation_frame = 0;
static uint8_t static_animation_frame = 0;
static uint8_t static_animation_rand_timer = 60;
static uint8_t static_animation_rand_value = 0;
static float static_animation_alpha = 0.5f;

enum {
	CS_CLOSED = 0,
	CS_OPENING,
	CS_OPENED,
	CS_CLOSING
};

static uint32_t sprite_shader_program;
static uint8_t office_view_sprite_state = 0;
static uint8_t night_current = 1;
static uint8_t door_button_flags = 0;
static uint8_t power_usage_value;
static float fan_animation_frame = 0.0f;
static float door_frame_timers[2] = {0.0f};
static float power_left_value = 99.9f;
static float hour_timer = 0.0f;
static uint8_t camera_bar_hovering = 0;
static uint8_t camera_state = CS_CLOSED;
static uint8_t camera_selected = 0;
static float camera_flip_timer = CAM_TIMER_INIT;

static float title_timer1 = 0.0f;
static float title_timer2 = 0.0f;
static float title_blip_alpha = 0.0f;
static uint8_t title_blip_visible = 0.0f;
static float title_face_alpha = 0.0f;
static uint8_t title_face_glitch = 0;

static uint8_t light_flicker;

static float office_look_current = -160.0f;
static uint8_t space_pressed = 0;

static float camera_look_current = 0.0f;
static float camera_look_hold_timer = 0.0f;
static uint8_t camera_look_state = 0;

static uint8_t menu_option_selected = 0;
static uint8_t menu_option_hover_old = 0;
static float menu_selector_ypos = 404.0f;

static mat4 matrix_projection;

enum {
	GS_TITLE,
	GS_GAME,
};

static uint8_t game_state = GS_TITLE;

int main() {
	/* load GLFW */
	#ifdef DEBUG
		if(!glfwInit()) {
			printf("ERROR: GLFW fucked up.\n");
			return 1;
		}
	#else
		glfwInit();
	#endif

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

	/* create window */
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Five Nights at Freddy's", NULL, NULL);
	#ifdef DEBUG
		if(!window) {
			printf("ERROR: Window fucked up.\n");
			return 1;
		}
	#endif

	{ /* get monitor properties */
		GLFWmonitor *monitor;
		ivec2 monitor_size;
		int32_t monitor_count;
		monitor = *glfwGetMonitors(&monitor_count);
		#ifdef DEBUG
			if(!monitor) {
				printf("ERROR: Monitor fucked up.\n");
				glfwDestroyWindow(window);
				glfwTerminate();
				return 1;
			}
		#endif

		glfwGetMonitorWorkarea(monitor, NULL, NULL, &monitor_size[0], &monitor_size[1]);
		glfwSetWindowPos(window, (monitor_size[0] / 2) - (WINDOW_WIDTH / 2), (monitor_size[1] / 2) - (WINDOW_HEIGHT / 2));
	}

	glfwMakeContextCurrent(window);

	/* load GLAD */
	#ifdef DEBUG
		if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			printf("ERROR: GLAD fucked up.\n");
			glfwDestroyWindow(window);
			glfwTerminate();
			return 1;
		}
	#else
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	#endif

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	/* set up matricies */
	glm_ortho(0.0f, WINDOW_WIDTH, 0.0f, WINDOW_HEIGHT, -1.0f, 1.0f, matrix_projection);

	/* create shaders */
	render_shader_program = shader_create("resources/shaders/render_vertex.glsl", "resources/shaders/render_fragment.glsl");
	ui_shader_program = shader_create("resources/shaders/render_ui_vertex.glsl", "resources/shaders/render_ui_fragment.glsl");
	sprite_shader_program = shader_create("resources/shaders/sprite_vertex.glsl", "resources/shaders/sprite_fragment.glsl");

	sound_system_create();

	/* load assets */
	assets_global = assets_global_create();
	switch(game_state) {
		case GS_TITLE:
			assets_title = assets_title_create();
			sound_play(assets_global.blip_sound);
			sound_play(assets_global.static_sound);
			sound_play(assets_title.music);
			office_look_current = 0.0f;
			camera_look_current = 0.0f;
			break;

		case GS_GAME:
			assets_game = assets_game_create();
			sound_play(assets_game.fan_sound);
			sound_play(assets_game.light_sound);

			camera_state = CS_CLOSED;
			camera_selected = 0;
			door_button_flags = 0;
			hour_timer = 0.0f;
			power_left_value = 99.9f;
			office_look_current = -160.0f;
			camera_look_current = 0.0f;
			break;
	}
	assets_print_loaded();

	/* set up audio listener */
	alListeneri(AL_DISTANCE_MODEL, AL_INVERSE_DISTANCE_CLAMPED);

	/* enable random shit */
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	#ifdef DEBUG
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("ERROR: Framebuffer fucked up.\n");
			return 1;
		}
	#endif

	{/* generate buffers for render texture */
		uint32_t render_vbo;
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
	srand((uint32_t)time(NULL));
	while(!glfwWindowShouldClose(window)) {
		ivec2 mouse_position;
		mat4 matrix_view;

		/* calculate deltatime */
		double time_now;
		float time_delta;
		float ticks;


		time_now = glfwGetTime();
		time_delta = (float)(time_now - time_last);
		time_last = time_now;
 		ticks = time_delta * 60.0f;

		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, 1);
		}

		#ifdef DEBUG
		{
			const float time_multiplier = (float)(glfwGetKey(window, GLFW_KEY_F) * TIME_MULTIPLIER) + 1.0f;
			time_delta *= time_multiplier;
			ticks *= time_multiplier;
			time_now *= (double)time_multiplier;
		}
		#endif

		if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !space_pressed) {
			game_state = !game_state;
			switch(game_state) {
				case GS_TITLE:
					sound_stop(assets_game.light_sound);
					sound_stop(assets_game.fan_sound);
					assets_game_destroy(&assets_game);

					assets_title = assets_title_create();
					sound_play(assets_global.blip_sound);
					sound_play(assets_global.static_sound);
					sound_play(assets_title.music);
					office_look_current = 0.0f;
					camera_look_current = 0.0f;
					break;

				case GS_GAME:
					sound_stop(assets_global.blip_sound);
					sound_stop(assets_global.static_sound);
					assets_title_destroy(&assets_title);

					assets_game = assets_game_create();
					sound_play(assets_game.fan_sound);
					sound_play(assets_game.light_sound);

					camera_state = CS_CLOSED;
					camera_selected = 0;
					door_button_flags = 0;
					hour_timer = 0.0f;
					power_left_value = 99.9f;
					office_look_current = -160.0f;
					camera_look_current = 0.0f;
					night_current++;
					break;
			}
		    space_pressed = 1;
			assets_print_loaded();
		}
		
		if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
		    space_pressed = 0;
		}

		mouse_get_position(window, mouse_position);

		/* update all animations */
		fan_animation_frame += ticks;
		fan_animation_frame = fmod2(fan_animation_frame, 3);

		/* update door animations */
		for(uint8_t i = 0; i < 2; i++) {
			if(door_button_flags & (DOOR_BUTTON_DOOR_FLAG << (i * 2))) {
				door_frame_timers[i] += ticks;
			} else {
				door_frame_timers[i] -= ticks;
			}
			door_frame_timers[i] = clampf(door_frame_timers[i], 0.0f, 28.0f);
		}

		scaled_update_timer += time_delta;
		title_timer1 += time_delta;
		title_timer2 += time_delta;
		if(scaled_update_timer > ANIMATION_FRAMETIME) {
			const uint8_t static_animation_frame_old = static_animation_frame;

			/* light flicker effect */
			float light_buzz_volume_new = 0.0f;
			office_view_sprite_state = 0;
			light_flicker = (uint8_t)(rand() % 10);
			for(uint8_t i = 0; i < 2; i++) {
				if(door_button_flags & (DOOR_BUTTON_LIGHT_FLAG << (i * 2))) {
					office_view_sprite_state = (i + 1) * (light_flicker > 1);
					light_buzz_volume_new = (float)(light_flicker > 1);
				}
			}

			if(blip_animation_frame < 9)
				blip_animation_frame++;

			/* static animation flicker */
			do {
				static_animation_frame = (uint8_t)rand() % 8;
			} while(static_animation_frame == static_animation_frame_old);

			static_animation_rand_timer--;
			if(static_animation_rand_timer == 0xFF) {
				static_animation_rand_value = ((uint8_t)rand() % 3) * 15;
				static_animation_rand_timer = 60;
			}
			static_animation_alpha = 1.0f - ((((game_state == GS_TITLE) ? 100.0f : 150.0f) + (rand() % 50) + static_animation_rand_value) / 255.0f);

			sound_set_gain(assets_game.light_sound, light_buzz_volume_new);
			scaled_update_timer = 0.0f;

			/* title glitchy blip flicker */
			if(title_timer1 > 0.08f) {
				title_blip_alpha = 1.0f - ((float)((rand() % 100) + 100) / 255.0f);
				title_face_glitch = (uint8_t)(rand() % 100);
				title_timer1 = 0.0f;
			}

			if(title_timer2 > 0.3f) {
				title_blip_visible = !(rand() % 3);
				title_face_alpha = 1.0f - ((float)(rand() % 250) / 255.0f);
				title_timer2 = 0.0f;
			}
		}

		glm_mat4_identity(matrix_view);
		switch(game_state) {
			case GS_TITLE: {
				const uint16_t glitchy_blip_frame = (uint16_t)(blink_timer_get_tick((float)time_now, 10.0f, 60.0f, 8.0f));

				/* updating */
				assets_title.scanline_sprite.position[1] = fmod2((float)time_now * 30.0f, 752.0f) - 32.0f;

				/* drawing */
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				glUseProgram(sprite_shader_program);
				glUniformMatrix4fv(glGetUniformLocation(sprite_shader_program, "view"), 1, GL_FALSE, (const GLfloat *)matrix_view);
				glUniformMatrix4fv(glGetUniformLocation(sprite_shader_program, "projection"), 1, GL_FALSE, (const GLfloat *)matrix_projection);
				glUniform1f(glGetUniformLocation(sprite_shader_program, "alpha"), 1.0f);
				glUniform1i(glGetUniformLocation(sprite_shader_program, "follow_camera"), 1);

				glUniform1f(glGetUniformLocation(sprite_shader_program, "alpha"), title_face_alpha);
				sprite_draw(assets_title.freddy_face_sprite, sprite_shader_program, title_face_glitch * (title_face_glitch < 4));

				glUniform1f(glGetUniformLocation(sprite_shader_program, "alpha"), 1.0f);
				sprite_draw(assets_title.name_sprite, sprite_shader_program, 0);

				glUniform1f(glGetUniformLocation(sprite_shader_program, "alpha"), 0.2156862f);
				sprite_draw(assets_title.scanline_sprite, sprite_shader_program, 0);
				glUniform1f(glGetUniformLocation(sprite_shader_program, "alpha"), 1.0f);

				for(uint8_t i = 0; i < 2; i++) {
					vec4 copyright_boxes[2] = {
						{1044.0f, 691.0f, 226.0f, 14.0f},
						{1044.0f, 668.0f, 225.0f, 19.0f},
					};

					glm_vec2((float *)copyright_boxes[i], assets_title.copyright_sprites.position);
					glm_vec2((float *)copyright_boxes[i] + 2, assets_title.copyright_sprites.size);
					sprite_draw(assets_title.copyright_sprites, sprite_shader_program, i);
				}

				{
					uint8_t menu_option_hover = 0;
					uint8_t menu_option_selected_old = menu_option_selected;
					ivec4 menu_option_boxes[4] = {
						{174, 404, 203, 33},
						{174, 475, 204, 34},
						{174, 549, 227, 44},
						{174, 617, 306, 44},
					};

					for(uint8_t i = 0; i < 4; i++) {
						if(mouse_inside_box(mouse_position, menu_option_boxes[i], 0.0f)) {
							menu_option_hover++;
							menu_option_selected = i;
							menu_selector_ypos = (float)menu_option_boxes[i][1];
							break;
						}
					}

					if(menu_option_hover && (menu_option_hover != menu_option_hover_old) && (menu_option_selected != menu_option_selected_old)) {
						sound_play(assets_global.blip_sound);
					}
					menu_option_hover_old = menu_option_hover;

					for(uint8_t i = 0; i < 4; i++) {
						assets_title.menu_option_sprites.position[0] = (float)menu_option_boxes[i][0];
						assets_title.menu_option_sprites.position[1] = (float)menu_option_boxes[i][1];
						assets_title.menu_option_sprites.size[0] = (float)menu_option_boxes[i][2];
						assets_title.menu_option_sprites.size[1] = (float)menu_option_boxes[i][3];
						sprite_draw(assets_title.menu_option_sprites, sprite_shader_program, i);
					}

					glm_vec2_copy((vec2){111.0f, menu_selector_ypos + 4.0f}, assets_title.menu_option_sprites.position);
					glm_vec2_copy((vec2){43.0f, 26.0f}, assets_title.menu_option_sprites.size);
					sprite_draw(assets_title.menu_option_sprites, sprite_shader_program, 5);
				}

				if(title_blip_visible) {
					glUniform1f(glGetUniformLocation(sprite_shader_program, "alpha"), title_blip_alpha);
					sprite_draw(assets_title.glitchy_blip, sprite_shader_program, glitchy_blip_frame);
					glUniform1f(glGetUniformLocation(sprite_shader_program, "alpha"), 1.0f);
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				glUseProgram(render_shader_program);
				glUniform1i(glGetUniformLocation(render_shader_program, "use_perspective"), 0);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, render_texture);
				glUniform1i(glGetUniformLocation(render_shader_program, "render_texture"), 0);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, assets_global.static_animation_sprite.textures[static_animation_frame]);
				glUniform1i(glGetUniformLocation(render_shader_program, "overlay_texture"), 1);
				glUniform1f(glGetUniformLocation(render_shader_program, "overlay_alpha"), static_animation_alpha);

				glBindVertexArray(render_vao);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				#ifdef DEBUG
				{
					char buffers[10][256];
					sprintf(buffers[0], "DEBUG MODE");
					sprintf(buffers[1], "    Scanline Y-Pos: %.0f", (double)assets_title.scanline_sprite.position[1]);
					sprintf(buffers[2], "    Blip Alpha: %.2f", (double)title_blip_alpha);
					sprintf(buffers[3], "    Blip Visible: %u", title_blip_visible);
					sprintf(buffers[4], "    Static Frame: %.0f", (double)static_animation_frame);
					sprintf(buffers[5], "    Static Alpha: %.2f", (double)static_animation_alpha);
					sprintf(buffers[6], "    Mouse Position: (%.0f, %.0f)\n", (double)mouse_position[0], (double)mouse_position[1]);
					sprintf(buffers[7], "    Option Selected: %u\n", menu_option_selected);
					sprintf(buffers[8], "    Time Passed: %.2f", time_now);
					sprintf(buffers[9], "    FPS: %.0f", (1.0 / (double)time_delta));

					for(uint8_t i = 0; i < 10; i++)
						font_draw(assets_global.debug_font, buffers[i], (vec2){WINDOW_WIDTH - 64.0f, WINDOW_HEIGHT + 256.0f - (48.0f * i)}, GLM_VEC3_ONE, 0.6f);
				}
				#endif

				break;
			}

			case GS_GAME: {
				vec2 camera_button_positions[11] = {
					{983.0f, 353.0f},
					{963.0f, 409.0f},
					{931.0f, 487.0f},
					{983.0f, 603.0f},
					{983.0f, 643.0f},
					{899.0f, 585.0f},
					{1089.0f, 604.0f},
					{1089.0f, 644.0f},
					{857.0f, 436.0f},
					{1186.0f, 568.0f},
					{1195.0f, 437.0f},
				};

				/* use the appropriate room scroll setting */
				if(camera_state != CS_OPENED) {
					/* default room turning */
					float mouse_distance_from_center = (float)mouse_position[0] - (WINDOW_WIDTH / 2.0f);
					mouse_distance_from_center = clampf(mouse_distance_from_center, -640.0f, 640.0f);
					mouse_distance_from_center *= !(fabsf(mouse_distance_from_center) < 128.0f);

					office_look_current += -mouse_distance_from_center * time_delta;
					office_look_current = clampf(office_look_current, -320.0f, 0.0f);
					/*
					// custom room turning
					float office_look_target;
					float mouse_normalized_x = (float)mouse_position[0] / (float)WINDOW_WIDTH;

					mouse_normalized_x = clampf(mouse_normalized_x, 0.0f, 1.0f);
					office_look_target = mouse_normalized_x * -320;
					office_look_current += (office_look_target - office_look_current) * time_delta * 8.0f;
					*/
				}

				/* camera flipping */
				{
					const uint8_t camera_state_old = camera_state;
					if(mouse_inside_box(mouse_position, (ivec4){75, 653, 792, 67}, 0.0f)) {
						if(!camera_bar_hovering) {
							camera_bar_hovering = 1;

							switch(camera_state) {
								case CS_CLOSED:
									camera_state = CS_OPENING;
									sound_stop(assets_game.camera_close_sound);
									sound_play(assets_game.camera_open_sound);
									sound_play(assets_game.camera_scan_sound);

									break;

								case CS_OPENED:
									camera_state = CS_CLOSING;
									sound_stop(assets_game.camera_open_sound);
									sound_stop(assets_game.camera_scan_sound);
									sound_play(assets_game.camera_close_sound);

									sound_set_gain(assets_game.fan_sound, 0.25f);
									break;

								default:
									break;
							}
						}
					}

					camera_bar_hovering *= !(mouse_position[1] < 643.0);

					if(camera_state == CS_OPENING || camera_state == CS_CLOSING) {
						if(camera_flip_timer > 0.0f) {
							camera_flip_timer -= time_delta;
						} else {
							camera_state++;
							camera_state %= 4;
							camera_flip_timer = CAM_TIMER_INIT;
						}
					}

					if(camera_state == CS_OPENED && camera_state_old != CS_OPENED) {
						door_button_flags &= DOOR_BUTTON_BOTH_DOORS_FLAG;
						sound_play(assets_global.blip_sound);
						blip_animation_frame = 0;
						sound_set_gain(assets_game.fan_sound, 0.1f);
					}
				}

				{ /* camera moving */
					/* TODO: Maybe optimize this bullshit */
					if(camera_look_hold_timer > 0.0f) {
						camera_look_hold_timer -= time_delta;
					} else {
						if(camera_look_state) {
							camera_look_current += time_delta * 60.0f;
							if(camera_look_current > 0.0f) {
								camera_look_current = 0.0f;
								camera_look_hold_timer = 1.6666667f;
								camera_look_state = !camera_look_state;
							}
						} else {
							camera_look_current -= time_delta * 60.0f;
							if(camera_look_current < -320.0f) {
								camera_look_current = -320.0f;
								camera_look_hold_timer = 1.6666667f;
								camera_look_state = !camera_look_state;
							}
						}
					}
				}

				/* check for clicking door buttons */
				if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !mouse_has_clicked) {
					const uint8_t door_button_flags_old = door_button_flags;
					const int32_t mouse_offset = (int32_t)office_look_current;

					if(camera_state != CS_OPENED) {
						const ivec4 door_button_boxes[4] = {{27, 251, 62, 120}, {1519, 267, 62, 120}, {25, 393, 62, 120}, {1519, 398, 62, 120}};
						for(uint8_t i = 0; i < 2; i++) {
							uint8_t door_button_bit_mask = (uint8_t)(DOOR_BUTTON_DOOR_FLAG << (i * 2));
							if(!((uint8_t)door_frame_timers[i]) || (uint8_t)door_frame_timers[i] == 28) {
								if(mouse_inside_box(mouse_position, door_button_boxes[i], mouse_offset)) {
									door_button_flags ^= door_button_bit_mask;
									sound_play(assets_game.door_sound);
								}
							}

							door_button_flags ^= (door_button_bit_mask >> 1) * mouse_inside_box(mouse_position, door_button_boxes[i + 2], mouse_offset);
						}

						/* pressing Freddy's nose */
						if(mouse_inside_box(mouse_position, (ivec4){674, 236, 8, 8}, mouse_offset)) {
							sound_stop(assets_game.freddy_nose_sound);
							sound_play(assets_game.freddy_nose_sound);
						}
					} else {
						/* selecting different cameras */
						for(uint8_t i = 0; i < 11; i++) {
							ivec4 camera_button_box_current;
							camera_button_box_current[0] = (int32_t)camera_button_positions[i][0] - 29;
							camera_button_box_current[1] = (int32_t)camera_button_positions[i][1] - 19;
							camera_button_box_current[2] = 60.0f;
							camera_button_box_current[3] = 40.0f;
								/*
							for(uint8_t j = 0; j < 4; j++) {
								//camera_button_box_current[j] = (int32_t)camera_button_positions[i][(int32_t)((float)j / 2.0f)];
							}
							*/
							// glm_ivec4_sub(camera_button_box_current, (ivec4){29, -31, 19, -21}, camera_button_box_current);

							if(mouse_inside_box(mouse_position, camera_button_box_current, 0.0f)) {
								sound_play(assets_global.blip_sound);
								blip_animation_frame = 0;
								camera_selected = i;
							}
						}
					}

					/* handle cases where both lights are toggled */
					if(door_button_flags & DOOR_BUTTON_LIGHT_FLAG) {
						if(door_button_flags_old & (DOOR_BUTTON_LIGHT_FLAG << 2)) {
							door_button_flags &= ~(DOOR_BUTTON_LIGHT_FLAG << 2);
						}
					}

					if(door_button_flags & (DOOR_BUTTON_LIGHT_FLAG << 2)) {
						if(door_button_flags_old & DOOR_BUTTON_LIGHT_FLAG) {
							door_button_flags &= ~DOOR_BUTTON_LIGHT_FLAG;
						}
					}

					mouse_has_clicked = 1;
				}

				/* power usage */
				power_usage_value = 0;
				for(uint8_t i = 0; i < 2; i++) {
					power_usage_value += ((door_button_flags >> (i * 2)) & 0x1) + ((((door_button_flags >> (i * 2)) & 0x2) > 0));
				}
				power_usage_value += camera_state == CS_OPENED;

				power_left_value -= ((float)power_usage_value + 1.0f) * time_delta * 0.1f;

				if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE) {
					mouse_has_clicked = 0;
				}

				glm_translate(matrix_view, (vec3){(camera_state == CS_OPENED) ? camera_look_current : office_look_current, 0.0f, 0.0f});

				/* draw */
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				/* draw office */
				glUseProgram(sprite_shader_program);
				glUniformMatrix4fv(glGetUniformLocation(sprite_shader_program, "view"), 1, GL_FALSE, (const GLfloat *)matrix_view);
				glUniformMatrix4fv(glGetUniformLocation(sprite_shader_program, "projection"), 1, GL_FALSE, (const GLfloat *)matrix_projection);
				glUniform1i(glGetUniformLocation(sprite_shader_program, "follow_camera"), 0);

				if(camera_state != CS_OPENED) {
					sprite_draw(assets_game.office_view_sprite, sprite_shader_program, office_view_sprite_state);
					sprite_draw(assets_game.fan_animation_sprite, sprite_shader_program, (uint8_t)fan_animation_frame);

					for(uint8_t i = 0; i < 2; i++) {
						sprite_draw(assets_game.door_animation_sprites[i], sprite_shader_program, (uint8_t)(door_frame_timers[i] / 2));
						glUniform1i(glGetUniformLocation(sprite_shader_program, "flip_x"), !i);
					}

					for(uint8_t i = 0; i < 2; i++) {
						sprite_draw(assets_game.door_button_sprites[i], sprite_shader_program, (door_button_flags >> (2 * i)) & 0x3);
					}
				} else {
					const uint8_t camera_selected_offsets[11] = { 0, 7, 13, 18, 54, 60, 62, 68, 77, 0, 81 };
					if(camera_selected != 9) {
						sprite_draw(assets_game.camera_view_sprite, sprite_shader_program, camera_selected_offsets[camera_selected] + ((light_flicker <= 3) * camera_selected == 3));
					}
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				glUseProgram(render_shader_program);
				glUniform1i(glGetUniformLocation(render_shader_program, "use_perspective"), 1);
				glUniform1i(glGetUniformLocation(render_shader_program, "render_texture"), 0);
				glUniform1f(glGetUniformLocation(render_shader_program, "overlay_alpha"), 1.0f);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, render_texture);
				glBindVertexArray(render_vao);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				/* ui elements */
				glUseProgram(ui_shader_program);
				glUniformMatrix4fv(glGetUniformLocation(ui_shader_program, "projection"), 1, GL_FALSE, (const GLfloat *)matrix_projection);
				glUniform1f(glGetUniformLocation(ui_shader_program, "alpha"), 1.0f);

				// printf("%.1f\n", (double)fmod2((float)time_now * 60.0f, 1.0f));
				if(camera_state == CS_OPENED) {
					uint8_t blink_state_dot;
					uint8_t blink_state_buttons;
					float blink_timer_dot = blink_timer_get_tick((float)time_now, 2.0f, 60.0f, 1.0f);
					float blink_timer_buttons = blink_timer_get_tick((float)time_now, 3.0f, 60.0f, 1.0f);
					const float camera_view_name_widths[11] = { 217.0f, 239.0f, 228.0f, 192.0f, 305.0f, 284.0f, 192.0f, 305.0f, 195.0f, 151.0f, 196.0f };
	
					blink_state_dot = blink_timer_dot < 0.5f;
					blink_state_buttons = blink_timer_buttons < 0.5f;

					glUniform1f(glGetUniformLocation(ui_shader_program, "alpha"), static_animation_alpha);
					sprite_draw(assets_global.static_animation_sprite, ui_shader_program, static_animation_frame);
					glUniform1f(glGetUniformLocation(ui_shader_program, "alpha"), 1.0f);

					if(blip_animation_frame < 9)
						sprite_draw(assets_global.blip_animation_sprite, ui_shader_program, blip_animation_frame);

					sprite_draw(assets_game.camera_border_sprite, ui_shader_program, 0);
					sprite_draw(assets_game.camera_map_sprite, ui_shader_program, blink_state_dot);

					/* draw camera name */
					assets_game.camera_view_name_sprite.size[0] = camera_view_name_widths[camera_selected];
					sprite_draw(assets_game.camera_view_name_sprite, ui_shader_program, camera_selected);

					/* draw all camera buttons */
					for(uint8_t i = 0; i < 11; i++) {
						vec2 camera_button_position_current;
						glm_vec2_sub(camera_button_positions[i], (vec2){29.0f, 19.0f}, camera_button_position_current);
						glm_vec2_copy(camera_button_position_current, assets_game.camera_button_sprite.position);
						sprite_draw(assets_game.camera_button_sprite, ui_shader_program, blink_state_buttons * (camera_selected == i));

						glm_vec2_sub(camera_button_positions[i], (vec2){22.0f, 12.0f}, camera_button_position_current);
						glm_vec2_copy(camera_button_position_current, assets_game.camera_button_name_sprite.position);
						sprite_draw(assets_game.camera_button_name_sprite, ui_shader_program, i);
					}

					if(blink_state_dot)
						sprite_draw(assets_game.camera_recording_sprite, ui_shader_program, 0);

					if(camera_selected == 9) {
						sprite_draw(assets_game.camera_disabled_sprite, ui_shader_program, 0);
					}
				}

				sprite_draw(assets_game.power_usage_text_sprite, ui_shader_program, 0);
				sprite_draw(assets_game.power_usage_sprite, ui_shader_program, power_usage_value);

				sprite_draw(assets_game.power_left_sprite, ui_shader_program, 0);
				sprite_draw(assets_game.power_left_percent_sprite, ui_shader_program, 0);

				sprite_draw(assets_global.night_text_sprite, ui_shader_program, 0);
				sprite_draw(assets_global.night_number_sprite, ui_shader_program, night_current - 1);

				sprite_draw(assets_game.hour_am_sprite, ui_shader_program, 0);

				hour_timer += time_delta / 90.0f;
				if(hour_timer >= 6.0f) {
					/* TODO: Eventually add code to advance the night */
					printf("Congratulations! You survived to 6 AM!\n");
					glfwSetWindowShouldClose(window, 1);
				}

				if(hour_timer < 1.0f) {
					for(uint8_t i = 0; i < 2; i++) {
						glm_vec2_copy((vec2){1161 - (!i * 24), 29}, assets_game.hour_number_sprite.position);
						sprite_draw(assets_game.hour_number_sprite, ui_shader_program, i);
					}
				} else {
					glm_vec2_copy((vec2){1161, 29}, assets_game.hour_number_sprite.position);
					sprite_draw(assets_game.hour_number_sprite, ui_shader_program, (uint8_t)hour_timer - 1);
				}

				{
					uint8_t numbers_to_draw = (power_left_value >= 10.0f) + 1;
					for(uint8_t i = 0; i < numbers_to_draw; i++) {
						glm_vec2_copy((vec2){203 - (i * 18), 624}, assets_game.power_left_number_sprite.position);
						sprite_draw(assets_game.power_left_number_sprite, ui_shader_program, (uint8_t)(power_left_value / powf(10, i)) % 10);
					}
				}

				if((camera_state == CS_CLOSED || camera_state == CS_OPENED)) {
					if(!camera_bar_hovering) {
						sprite_draw(assets_game.camera_flip_bar_sprite, ui_shader_program, 0);
					}
				} else {
					sprite_draw(assets_game.camera_flip_animation_sprite, ui_shader_program, (uint16_t)(clampf(fabsf((10.0f * (camera_state == CS_OPENING)) - ((camera_flip_timer * (1 / CAM_TIMER_INIT)) * 10.0f)), 0.0f, 10.0f)));
				}

				#ifdef DEBUG
				{
					char buffers[9][256];
					sprintf(buffers[0], "DEBUG MODE");
					sprintf(buffers[1], "    Office Look: %.0f", (double)office_look_current);
					sprintf(buffers[2], "    Camera Look: %.0f", (double)camera_look_current);
					sprintf(buffers[3], "    Door Flags: %u%u%u%u", (door_button_flags) & 1, (door_button_flags >> 1) & 1, (door_button_flags >> 2) & 1, (door_button_flags >> 3) & 1);
					sprintf(buffers[4], "    Camera Flip State: %u", camera_state);
					sprintf(buffers[5], "    Camera Selected: %u", camera_selected);
					sprintf(buffers[6], "    Night Progress: %i%%", (int32_t)((hour_timer / 6.0f) * 100.0f));
					sprintf(buffers[7], "    Time Passed: %.2f", time_now);
					sprintf(buffers[8], "    FPS: %.0f", (double)(1.0f / time_delta));

					for(uint8_t i = 0; i < 9; i++) {
						font_draw(assets_global.debug_font, buffers[i], (vec2){64.0f, WINDOW_HEIGHT + 256.0f - (48.0f * i)}, GLM_VEC3_ONE, 0.6f);
					}
				}
				#endif

				break;
			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	/* destroy everything */
	glDeleteFramebuffers(1, &fbo);
	assets_game_destroy(&assets_game);
	assets_title_destroy(&assets_title);
	assets_global_destroy(&assets_global);
	sound_system_destroy();

	glDeleteShader(sprite_shader_program);
	glDeleteShader(ui_shader_program);
	glDeleteShader(render_shader_program);

	glfwTerminate();
	return 0;
}
