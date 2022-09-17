#include "assets.h"
#include "font.h"
#include "sound.h"

#include <assert.h>

static uint8_t global_loaded = 0;
static uint8_t title_loaded = 0;
static uint8_t game_loaded = 0;

assets_global_t assets_global_create() {
	assets_global_t a;
	assert(!global_loaded);

	font_shader_create();
	a.night_text_sprite = sprite_create((vec2){1148, 74}, (vec2){63, 14}, "resources/graphics/ui/night/night.png", 1);
	a.night_number_sprite = sprite_create((vec2){1223, 72}, (vec2){14, 17}, "resources/graphics/ui/night/", 7);
	a.static_animation_sprite = sprite_create(GLM_VEC2_ZERO, (vec2){1280.0f, 720.0f}, "resources/graphics/general/static/", 8);
	a.blip_animation_sprite = sprite_create(GLM_VEC2_ZERO, (vec2){1280.0f, 720.0f}, "resources/graphics/general/blip/", 9);
	a.black_sprite = sprite_create(GLM_VEC2_ZERO, (vec2){1600.0f, 720.0f}, "resources/graphics/black.png", 1);

	a.blip_sound = sound_create("resources/audio/sounds/blip.wav", 1.0f, 1.0f, GLM_VEC3_ZERO, 0);
	a.static_sound = sound_create("resources/audio/sounds/static.wav", 1.0f, 1.0f, GLM_VEC3_ZERO, 0);

	a.debug_font = font_create("resources/fonts/minecraftia.ttf");

	global_loaded = 1;
	return a;
}

void assets_global_destroy(assets_global_t *a) {
	if(!global_loaded)
		return;

	font_destroy(&a->debug_font);
	font_shader_destroy();

	sound_destroy(&a->static_sound);
	sound_destroy(&a->blip_sound);

	sprite_destroy(&a->black_sprite);
	sprite_destroy(&a->blip_animation_sprite);
	sprite_destroy(&a->static_animation_sprite);
	sprite_destroy(&a->night_number_sprite);
	sprite_destroy(&a->night_text_sprite);

	global_loaded = 0;
}

assets_title_t assets_title_create(void) {
	assets_title_t a;
	assert(!title_loaded);

	a.music = sound_create("resources/audio/music/title-music.wav", 1.0f, 1.0f, GLM_VEC3_ZERO, 1);

	title_loaded = 1;
	return a;
}

void assets_title_destroy(assets_title_t *a) {
	if(!title_loaded)
		return;

	sound_destroy(&a->music);

	title_loaded = 0;
}

assets_game_t assets_game_create() {
	assets_game_t a;
	vec2 door_positions[2] = {{72.0f, -1.0f}, {1270.0f, -2.0f}};
	assert(!game_loaded);

	for(uint8_t i = 0; i < 2; i++)
		a.door_animation_sprites[i] = sprite_create(door_positions[i], (vec2){223.0f, 720.0f}, "resources/graphics/office/doors/", 15);
	a.office_view_sprite = sprite_create(GLM_VEC2_ZERO, (vec2){1600.0f, 720.0f}, "resources/graphics/office/states/", 5);
	a.camera_view_sprite = sprite_create(GLM_VEC2_ZERO, (vec2){1600.0f, 720.0f}, "resources/graphics/camera/", 85);
	a.camera_view_name_sprite = sprite_create((vec2){832.0f, 292.0f}, (vec2){239.0f, 26.0f}, "resources/graphics/ui/camera/map/names/", 11);
	a.fan_animation_sprite = sprite_create((vec2){780.0f, 303.0f}, (vec2){137.0f, 196.0f}, "resources/graphics/office/fan/", 3);
	a.door_button_sprites[0] = sprite_create((vec2){6.0f, 263.0f}, (vec2){92.0f, 247.0f}, "resources/graphics/office/doors/buttons/l", 4);
	a.door_button_sprites[1] = sprite_create((vec2){1497.0f, 273.0f}, (vec2){92.0f, 247.0f}, "resources/graphics/office/doors/buttons/r", 4);
	a.power_usage_sprite = sprite_create((vec2){120, 657}, (vec2){103, 32}, "resources/graphics/ui/power/levels/", 4);
	a.power_usage_text_sprite = sprite_create((vec2){38, 667}, (vec2){72, 14}, "resources/graphics/ui/power/usage.png", 1);
	a.power_left_sprite = sprite_create((vec2){38, 631}, (vec2){137, 14}, "resources/graphics/ui/power/power-left-0.png", 1);
	a.power_left_percent_sprite = sprite_create((vec2){228, 632}, (vec2){11, 14}, "resources/graphics/ui/power/power-left-1.png", 1);
	a.power_left_number_sprite = sprite_create(GLM_VEC2_ZERO, (vec2){18, 22}, "resources/graphics/ui/power/numbers/", 10);
	a.hour_am_sprite = sprite_create((vec2){1200, 31}, (vec2){42, 26}, "resources/graphics/ui/am.png", 1);
	a.hour_number_sprite = sprite_create((vec2){1161, 29}, (vec2){24, 30}, "resources/graphics/ui/hour/", 6);
	a.camera_flip_bar_sprite = sprite_create((vec2){255, 638}, (vec2){600, 60}, "resources/graphics/ui/camera/bar.png", 1);
	a.camera_flip_animation_sprite = sprite_create(GLM_VEC2_ZERO, (vec2){1280, 720}, "resources/graphics/ui/camera/flip/", 11);
	a.camera_border_sprite = sprite_create(GLM_VEC2_ZERO, (vec2){1280, 720}, "resources/graphics/ui/camera/border.png", 1);
	a.camera_map_sprite = sprite_create((vec2){848.0f, 313.0f}, (vec2){400.0f, 400.0f}, "resources/graphics/ui/camera/map/", 2);
	a.camera_recording_sprite = sprite_create((vec2){68.0f, 52.0f}, (vec2){50.0f, 50.0f}, "resources/graphics/ui/camera/recording-dot.png", 1);
	a.camera_button_sprite = sprite_create(GLM_VEC2_ZERO, (vec2){60.0f, 40.0f}, "resources/graphics/ui/camera/map/button/", 2);
	a.camera_button_name_sprite = sprite_create(GLM_VEC2_ZERO, (vec2){31.0f, 25.0f}, "resources/graphics/ui/camera/map/button/text/", 11);
	a.camera_disabled_sprite = sprite_create((vec2){464.0f, 69.0f}, (vec2){371.0f, 54.0f}, "resources/graphics/ui/camera/map/disabled.png", 1);

	a.fan_sound = sound_create("resources/audio/sounds/fan.wav", 1.0f, 0.25f, GLM_VEC3_ZERO, 1);
	a.light_sound = sound_create("resources/audio/sounds/light-hum.wav", 1.0f, 0.0f, GLM_VEC3_ZERO, 1);
	a.door_sound = sound_create("resources/audio/sounds/door-activate.wav", 1.0f, 1.0f, GLM_VEC3_ZERO, 0);
	a.freddy_nose_sound = sound_create("resources/audio/sounds/boop.wav", 1.0f, 0.4f, GLM_VEC3_ZERO, 0);
	a.camera_open_sound = sound_create("resources/audio/sounds/cam-open.wav", 1.0f, 1.0f, GLM_VEC3_ZERO, 0);
	a.camera_scan_sound = sound_create("resources/audio/sounds/cam-scan.wav", 1.0f, 1.0f, GLM_VEC3_ZERO, 0);
	a.camera_close_sound = sound_create("resources/audio/sounds/cam-close.wav", 1.0f, 1.0f, GLM_VEC3_ZERO, 0);

	game_loaded = 1;

	return a;
}

void assets_game_destroy(assets_game_t *a) {
	if(!game_loaded)
		return;

	sound_destroy(&a->camera_close_sound);
	sound_destroy(&a->camera_scan_sound);
	sound_destroy(&a->camera_open_sound);
	sound_destroy(&a->freddy_nose_sound);
	sound_destroy(&a->door_sound);
	sound_destroy(&a->light_sound);
	sound_destroy(&a->fan_sound);

	sprite_destroy(&a->camera_disabled_sprite);
	sprite_destroy(&a->camera_button_name_sprite);
	sprite_destroy(&a->camera_button_sprite);
	sprite_destroy(&a->camera_recording_sprite);
	sprite_destroy(&a->camera_map_sprite);
	sprite_destroy(&a->camera_border_sprite);
	sprite_destroy(&a->camera_flip_animation_sprite);
	sprite_destroy(&a->camera_flip_bar_sprite);
	sprite_destroy(&a->hour_number_sprite);
	sprite_destroy(&a->hour_am_sprite);
	sprite_destroy(&a->power_left_number_sprite);
	sprite_destroy(&a->power_left_percent_sprite);
	sprite_destroy(&a->power_left_sprite);
	sprite_destroy(&a->power_usage_text_sprite);
	sprite_destroy(&a->power_usage_sprite);
	sprite_destroy(&a->door_button_sprites[1]);
	sprite_destroy(&a->door_button_sprites[0]);
	sprite_destroy(&a->fan_animation_sprite);
	sprite_destroy(&a->camera_view_name_sprite);
	sprite_destroy(&a->camera_view_sprite);
	sprite_destroy(&a->office_view_sprite);
	sprite_destroy(&a->door_animation_sprites[1]);
	sprite_destroy(&a->door_animation_sprites[0]);

	game_loaded = 0;
}

void assets_print_loaded() {
	printf("GLOBAL: %u, TITLE: %u, GAME: %u\n", global_loaded, title_loaded, game_loaded);
}
