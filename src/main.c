#include <stdio.h>
#include <stdlib.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include <AL/al.h>
#include <AL/alc.h>

#include "file.h"
#include "texture.h"
#include "sprite.h"
#include "shader.h"
#include "mouse.h"

#include "sound.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define WINDOW_WIDTH					1280
#define WINDOW_HEIGHT					720

#define ANIMATION_FRAMETIME				0.01666667f

#define DOOR_BUTTON_LIGHT_FLAG			0x1
#define DOOR_BUTTON_DOOR_FLAG			0x2

#define CAM_TIMER_INIT 					0.35f

static GLFWwindow *window;
static ALCdevice *sound_device;
static ALCcontext *sound_context;
static uint8_t mouse_has_clicked = 0;

static float scaled_update_timer = 0.0f;

static uint32_t fbo;
static uint32_t rbo;
static uint32_t render_vao;

static double time_last;

static uint32_t render_texture;
static uint32_t render_shader_program;
static uint32_t render_ui_shader_program;

/* sprites and textures */
static sprite_t office_sprite;

static sprite_t fan_animation_sprite;
static float fan_animation_frame = 0.0f;

static sprite_t door_button_sprites[2];
static uint8_t door_button_flags = 0;

static sprite_t door_animation_sprites[2];
static float door_frame_timers[2] = {0.0f};

static uint32_t sprite_shader_program;
static uint8_t office_sprite_state = 0;

static sprite_t power_usage_sprite;
static uint8_t power_usage_value;
static sprite_t power_usage_text_sprite;

static sprite_t power_left_sprite;
static sprite_t power_left_percent_sprite;
static sprite_t power_left_number_sprite;
static float power_left_value = 99.9f;

static sprite_t hour_am_sprite;
static sprite_t hour_number_sprite;
static float hour_timer = 0.0f;

static sprite_t night_text_sprite;
static sprite_t night_number_sprite;

static enum {
	CAM_STATE_CLOSED = 0,
	CAM_STATE_OPENING,
	CAM_STATE_OPENED,
	CAM_STATE_CLOSING
};

static sprite_t cam_flip_bar_sprite;
static sprite_t cam_flip_animation_sprite;
static uint8_t cam_bar_hovering = 0;
static uint8_t cam_state = CAM_STATE_CLOSED;
static float cam_flip_timer = CAM_TIMER_INIT;

/* sound sources */
static uint32_t fan_sound_source;
static uint32_t light_sound_source;
static uint32_t door_sound_source;
static uint32_t freddy_nose_sound_source;

static uint32_t cam_open_sound_source;
static uint32_t cam_scan_sound_source;
static uint32_t cam_close_sound_source;
static uint32_t cam_blip_sound_source;

static uint32_t fan_sound_buffer;
static uint32_t light_sound_buffer;
static uint32_t door_sound_buffer;
static uint32_t freddy_nose_sound_buffer;

static uint32_t cam_open_sound_buffer;
static uint32_t cam_scan_sound_buffer;
static uint32_t cam_close_sound_buffer;
static uint32_t cam_blip_sound_buffer;

static uint32_t font_vao;
static uint32_t font_vbo;
static uint32_t font_shader_program;

static float office_look_current = 0.0f;
static uint8_t office_look_use_alternate = 0;
static uint8_t office_look_use_alternate_pressed = 0;

static mat4 matrix_projection;
static mat4 matrix_view;

float clampf(const float x, const float min, const float max) {
    float diff[2] = {min-x, x-max};
    uint32_t MemA = *(uint32_t*)&diff[0];
    uint32_t MemB = *(uint32_t*)&diff[1];
    uint8_t Flag_A = (MemA>>31);
    uint8_t Flag_B = (MemB>>31);
    uint8_t OOR = (Flag_A & Flag_B);
    return (OOR * x) + ((1-Flag_A) * min) + ((1-Flag_B) * max);
}

int main() {
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
		ivec2 monitor_size;
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

	/* create shaders */
	render_shader_program = shader_create("resources/shaders/render_vertex.glsl", "resources/shaders/render_fragment.glsl");
	render_ui_shader_program = shader_create("resources/shaders/render_ui_vertex.glsl", "resources/shaders/render_ui_fragment.glsl");
	font_shader_program = shader_create("resources/shaders/font_vertex.glsl", "resources/shaders/font_fragment.glsl");
	sprite_shader_program = shader_create("resources/shaders/sprite_vertex.glsl", "resources/shaders/sprite_fragment.glsl");

	{ /* load sprites */
		const vec2 door_positions[2] = {{72.0f, -1.0f}, {1270.0f, -2.0f}};
		for(uint8_t i = 0; i < 2; i++) {
			sprite_create(&door_animation_sprites[i], door_positions[i], (vec2){223.0f, 720.0f}, "resources/graphics/office/doors/", 15);
		}

		sprite_create(&door_button_sprites[0], (vec2){6.0f, 263.0f}, (vec2){92.0f, 247.0f}, "resources/graphics/office/doors/buttons/l", 4);
		sprite_create(&door_button_sprites[1], (vec2){1497.0f, 273.0f}, (vec2){92.0f, 247.0f}, "resources/graphics/office/doors/buttons/r", 4);
		sprite_create(&office_sprite, GLM_VEC2_ZERO, (vec2){1600.0f, 720.0f}, "resources/graphics/office/states/", 5);
		sprite_create(&fan_animation_sprite, (vec2){780.0f, 303.0f}, (vec2){137.0f, 196.0f}, "resources/graphics/office/fan/", 3);
		sprite_create(&power_usage_sprite, (vec2){120, 657}, (vec2){103, 32}, "resources/graphics/office/ui/power/levels/", 4);
		sprite_create(&power_usage_text_sprite, (vec2){38, 667}, (vec2){72, 14}, "resources/graphics/office/ui/power/usage.png", 1);
		sprite_create(&power_left_sprite, (vec2){38, 631}, (vec2){137, 14}, "resources/graphics/office/ui/power/power-left-0.png", 1);
		sprite_create(&power_left_percent_sprite, (vec2){228, 632}, (vec2){11, 14}, "resources/graphics/office/ui/power/power-left-1.png", 1);
		sprite_create(&power_left_number_sprite, GLM_VEC2_ZERO, (vec2){18, 22}, "resources/graphics/office/ui/power/numbers/", 10);
		sprite_create(&cam_flip_bar_sprite, (vec2){255, 638}, (vec2){600, 60}, "resources/graphics/office/ui/camera/bar.png", 1);
		sprite_create(&cam_flip_animation_sprite, GLM_VEC2_ZERO, (vec2){1280, 720}, "resources/graphics/office/ui/camera/flip/", 11);
		sprite_create(&hour_am_sprite, (vec2){1200, 31}, (vec2){42, 26}, "resources/graphics/office/ui/am.png", 1);
		sprite_create(&hour_number_sprite, (vec2){1161, 29}, (vec2){24, 30}, "resources/graphics/office/ui/hour/", 6);
		sprite_create(&night_text_sprite, (vec2){1148, 74}, (vec2){63, 14}, "resources/graphics/office/ui/night/night.png", 1);
		sprite_create(&night_number_sprite, (vec2){1223, 72}, (vec2){14, 17}, "resources/graphics/office/ui/night/", 7);
	}

	{ /* set up audio engine */
		const char *sound_device_name;
		sound_device = alcOpenDevice(NULL);
		if(!sound_device) {
		    printf("ERROR: Audio Device fucked up.");
		    return 1;
		}
		
		sound_context = alcCreateContext(sound_device, NULL);
		if(!sound_context) {
		    printf("ERROR: Audio Context fucked up.");
		    return 1;
		}
		
		if(!alcMakeContextCurrent(sound_context)) {
		    printf("ERROR: Making context fucked up.");
		    return 1;
		}
		
		sound_device_name = NULL;
		if(alcIsExtensionPresent(sound_device, "ALC_ENUMERATE_ALL_EXT")) {
		    sound_device_name = alcGetString(sound_device, ALC_ALL_DEVICES_SPECIFIER);
		}
		
		if(!sound_device_name || alcGetError(sound_device) != AL_NO_ERROR) {
		    sound_device_name = alcGetString(sound_device, ALC_DEVICE_SPECIFIER);
		}
		printf("SOUND DEVICE: %s\n", sound_device_name);
	}

	/* load sounds */
	fan_sound_buffer = sound_buffer_create("resources/audio/sounds/fan.wav");
	fan_sound_source = sound_source_create(fan_sound_buffer, 1.0f, 0.25f, GLM_VEC3_ZERO, 1);

	light_sound_buffer = sound_buffer_create("resources/audio/sounds/light-hum.wav");
	light_sound_source = sound_source_create(light_sound_buffer, 1.0f, 0.0f, GLM_VEC3_ZERO, 1);

	door_sound_buffer = sound_buffer_create("resources/audio/sounds/door-activate.wav");
	door_sound_source = sound_source_create(door_sound_buffer, 1.0f, 1.0f, GLM_VEC3_ZERO, 0);

	freddy_nose_sound_buffer = sound_buffer_create("resources/audio/sounds/boop.wav");
	freddy_nose_sound_source = sound_source_create(freddy_nose_sound_buffer, 1.0f, 0.4f, GLM_VEC3_ZERO, 0);

	cam_open_sound_buffer = sound_buffer_create("resources/audio/sounds/cam-open.wav");
	cam_open_sound_source = sound_source_create(cam_open_sound_buffer, 1.0f, 1.0f, GLM_VEC3_ZERO, 0);

	cam_scan_sound_buffer = sound_buffer_create("resources/audio/sounds/cam-scan.wav");
	cam_scan_sound_source = sound_source_create(cam_scan_sound_buffer, 1.0f, 1.0f, GLM_VEC3_ZERO, 0);

	cam_close_sound_buffer = sound_buffer_create("resources/audio/sounds/cam-close.wav");
	cam_close_sound_source = sound_source_create(cam_close_sound_buffer, 1.0f, 1.0f, GLM_VEC3_ZERO, 0);

	cam_blip_sound_buffer = sound_buffer_create("resources/audio/sounds/blip.wav");
	cam_blip_sound_source = sound_source_create(cam_blip_sound_buffer, 1.0f, 1.0f, GLM_VEC3_ZERO, 0);

	/* set up audio listener */
	alListeneri(AL_DISTANCE_MODEL, AL_INVERSE_DISTANCE_CLAMPED);

	/*
	{
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
	*/

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

	alSourcePlay(fan_sound_source);
	alSourcePlay(light_sound_source);

	/* main loop */
	srand(time(NULL));
	while(!glfwWindowShouldClose(window)) {
		ivec2 mouse_position;

		/* calculate deltatime */
		double time_now;
		float time_delta;
		time_now = glfwGetTime();
		time_delta = time_now - time_last;
		time_last = time_now;

		/* process input */
		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, 1);
		}

		if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !office_look_use_alternate_pressed) {
			office_look_use_alternate ^= 1;
			office_look_use_alternate_pressed = 1;
		}

		if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
			office_look_use_alternate_pressed = 0;
		}

		/* use the appropriate room scroll setting */
		mouse_get_position(window, mouse_position);
		if(cam_state != CAM_STATE_OPENED) {
			if(!office_look_use_alternate) {
				/* default room turning */
				float mouse_distance_from_center;
				mouse_distance_from_center = mouse_position[0] - (WINDOW_WIDTH / 2.0f);

				mouse_distance_from_center = clampf(mouse_distance_from_center, -640.0f, 640.0f);
				mouse_distance_from_center *= !(fabsf(mouse_distance_from_center) < 128.0f);

				office_look_current += -mouse_distance_from_center * time_delta;
				office_look_current = clampf(office_look_current, -320.0f, 0.0f);
			} else {
				/* custom room turning */
				float office_look_target;
				float mouse_normalized_x = (float)mouse_position[0] / (float)WINDOW_WIDTH;

				mouse_normalized_x = clampf(mouse_normalized_x, 0.0f, 1.0f);
				office_look_target = mouse_normalized_x * -320;
				office_look_current += (office_look_target - office_look_current) * time_delta * 8.0f;
			}
		}

		/* camera flipping */
		{
			const uint8_t cam_state_old = cam_state;
			if(mouse_inside_box(window, (ivec4){75, 75 + 792, 653, 653 + 67}, 0.0f)) {
				if(!cam_bar_hovering) {
					cam_bar_hovering = 1;
					switch(cam_state) {
						case CAM_STATE_CLOSED:
							cam_state = CAM_STATE_OPENING;
							alSourceStop(cam_close_sound_source);
							alSourcePlay(cam_open_sound_source);
							alSourcePlay(cam_scan_sound_source);
							break;

						case CAM_STATE_OPENED:
							cam_state = CAM_STATE_CLOSING;
							alSourceStop(cam_open_sound_source);
							alSourceStop(cam_scan_sound_source);
							alSourcePlay(cam_close_sound_source);
							break;

						default:
							break;
					}
				}
			} else {
				cam_bar_hovering = 0;
			}

			if((cam_state == CAM_STATE_OPENING || cam_state == CAM_STATE_CLOSING)) {
				if(cam_flip_timer > 0.0f) {
					cam_flip_timer -= time_delta;
				} else {
					cam_state++;
					cam_state %= 4;
					cam_flip_timer = CAM_TIMER_INIT;
				}
			}

			if(cam_state == CAM_STATE_OPENED && cam_state_old != CAM_STATE_OPENED) {
				door_button_flags &= 0b1010;
				alSourcePlay(cam_blip_sound_source);
			}
		}

		/* check for clicking door buttons */
		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !mouse_has_clicked) {
			const uint8_t door_button_flags_old = door_button_flags;
			const int32_t mouse_offset = (int32_t)office_look_current;

			if(cam_state != CAM_STATE_OPENED) {
				const ivec4 door_button_boxes[4] = {{27, 89, 251, 371}, {1519, 1581, 267, 387}, {25, 87, 393, 513}, {1519, 1581, 398, 518}};
				for(uint8_t i = 0; i < 2; i++) {
					uint8_t door_button_bit_mask = (DOOR_BUTTON_DOOR_FLAG << (i * 2));
					if(!((uint8_t)door_frame_timers[i]) || (uint8_t)door_frame_timers[i] == 28) {
						if(mouse_inside_box(window, door_button_boxes[i], mouse_offset)) {
							door_button_flags ^= door_button_bit_mask;
							alSourcePlay(door_sound_source);
						}
					}

					door_button_flags ^= (door_button_bit_mask >> 1) * mouse_inside_box(window, door_button_boxes[i + 2], mouse_offset);
				}

				/* pressing Freddy's nose */
				if(mouse_inside_box(window, (ivec4){674, 682, 236, 244}, mouse_offset)) {
					alSourceStop(freddy_nose_sound_source);
					alSourcePlay(freddy_nose_sound_source);
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
		power_usage_value += cam_state == CAM_STATE_OPENED;

		power_left_value -= ((float)power_usage_value + 1.0f) * time_delta * 0.1f;

		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE) {
			mouse_has_clicked = 0;
		}

		{ /* update */
			float ticks = time_delta * 60.0f;
			fan_animation_frame += ticks;
			fan_animation_frame = fmodf(fan_animation_frame, 3);

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
			if(scaled_update_timer > ANIMATION_FRAMETIME) {
				/* light flicker effect */
				float light_buzz_volume_new = 0.0f;
				uint8_t light_random = rand() % 10;
				office_sprite_state = 0;
				for(uint8_t i = 0; i < 2; i++) {
					if(door_button_flags & (DOOR_BUTTON_LIGHT_FLAG << (i * 2))) {
						office_sprite_state = (i + 1) * (light_random > 0);
						light_buzz_volume_new = clampf((float)light_random, 0.0f, 1.0f);
					}
				}
				alSourcef(light_sound_source, AL_GAIN, light_buzz_volume_new);
				scaled_update_timer = 0.0f;
			}
		}

		glm_mat4_copy(GLM_MAT4_IDENTITY, matrix_view);
		glm_translate(matrix_view, (vec3){office_look_current, 0.0f, 0.0f});

		/* draw */
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		/* draw office */
		glUseProgram(sprite_shader_program);
		glUniformMatrix4fv(glGetUniformLocation(sprite_shader_program, "view"), 1, GL_FALSE, (const GLfloat *)matrix_view);
		glUniformMatrix4fv(glGetUniformLocation(sprite_shader_program, "projection"), 1, GL_FALSE, (const GLfloat *)matrix_projection);

		sprite_draw(office_sprite, sprite_shader_program, office_sprite_state);

		if(cam_state != CAM_STATE_OPENED) {
			sprite_draw(fan_animation_sprite, sprite_shader_program, (uint8_t)fan_animation_frame);

			for(uint8_t i = 0; i < 2; i++) {
				sprite_draw(door_animation_sprites[i], sprite_shader_program, (uint8_t)(door_frame_timers[i] / 2));
				glUniform1i(glGetUniformLocation(sprite_shader_program, "flip_x"), !i);
			}

			for(uint8_t i = 0; i < 2; i++) {
				sprite_draw(door_button_sprites[i], sprite_shader_program, (door_button_flags >> (2 * i)) & 0b11);
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(render_shader_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, render_texture);
		glUniform1i(glGetUniformLocation(render_shader_program, "render_texture"), 0);
		glBindVertexArray(render_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		/* ui elements */
		glUseProgram(render_ui_shader_program);
		glUniformMatrix4fv(glGetUniformLocation(render_ui_shader_program, "projection"), 1, GL_FALSE, (const GLfloat *)matrix_projection);

		sprite_draw(power_usage_text_sprite, render_ui_shader_program, 0);
		sprite_draw(power_usage_sprite, render_ui_shader_program, power_usage_value);

		sprite_draw(power_left_sprite, render_ui_shader_program, 0);
		sprite_draw(power_left_percent_sprite, render_ui_shader_program, 0);

		sprite_draw(night_text_sprite, render_ui_shader_program, 0);
		sprite_draw(night_number_sprite, render_ui_shader_program, 0);

		sprite_draw(hour_am_sprite, render_ui_shader_program, 0);

		hour_timer += time_delta / 90.0f;
		if(hour_timer >= 6.0f) {
			/* TODO: Eventually add code to advance the night */
			printf("Congratulations! You survived to 6 AM!\n");
			glfwSetWindowShouldClose(window, 1);
		}

		if(hour_timer < 1.0f) {
			for(uint8_t i = 0; i < 2; i++) {
				sprite_set_position(&hour_number_sprite, (vec2){1161 - (!i * 24), 29});
				sprite_draw(hour_number_sprite, render_ui_shader_program, i);
			}
		} else {
			sprite_set_position(&hour_number_sprite, (vec2){1161, 29});
			sprite_draw(hour_number_sprite, render_ui_shader_program, (uint8_t)hour_timer - 1);
		}

		{
			uint8_t numbers_to_draw = (power_left_value >= 10.0f) + 1;
			for(uint8_t i = 0; i < numbers_to_draw; i++) {
				sprite_set_position(&power_left_number_sprite, (vec2){203 - (i * 18), 624});
				sprite_draw(power_left_number_sprite, render_ui_shader_program, (uint8_t)(power_left_value / powf(10, i)) % 10);
			}
		}

		if((cam_state == CAM_STATE_CLOSED || cam_state == CAM_STATE_OPENED)) {
			if(!cam_bar_hovering) {
				sprite_draw(cam_flip_bar_sprite, render_ui_shader_program, 0);
			}
		} else {
			sprite_draw(cam_flip_animation_sprite, render_ui_shader_program, (uint16_t)(fabsf((10.0f * (cam_state == CAM_STATE_OPENING)) - ((cam_flip_timer * (1 / CAM_TIMER_INIT)) * 10.0f))));
		}


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	/* destroy everything */
	glDeleteFramebuffers(1, &fbo);

	sprite_destroy(&night_number_sprite);
	sprite_destroy(&night_text_sprite);
	sprite_destroy(&hour_number_sprite);
	sprite_destroy(&hour_am_sprite);
	sprite_destroy(&cam_flip_animation_sprite);
	sprite_destroy(&cam_flip_bar_sprite);
	sprite_destroy(&power_left_number_sprite);
	sprite_destroy(&power_left_percent_sprite);
	sprite_destroy(&power_left_sprite);
	sprite_destroy(&power_usage_text_sprite);
	sprite_destroy(&power_usage_sprite);
	sprite_destroy(&fan_animation_sprite);
	sprite_destroy(&office_sprite);
	sprite_destroy(door_button_sprites + 1);
	sprite_destroy(door_button_sprites);

	glDeleteShader(sprite_shader_program);
	glDeleteShader(font_shader_program);
	glDeleteShader(render_ui_shader_program);
	glDeleteShader(render_shader_program);

	alDeleteSources(8, &fan_sound_source);
	alDeleteBuffers(8, &fan_sound_buffer);

	alcDestroyContext(sound_context);
	alcCloseDevice(sound_device);

	glfwTerminate();
	return 0;
}
