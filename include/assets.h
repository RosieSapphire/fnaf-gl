#ifndef ASSETS_H
#define ASSETS_H

#include "sprite.h"
#include "sound.h"

typedef struct {
	sprite_t night_text_sprite;
	sprite_t night_number_sprite;

	sprite_t blip_animation_sprite;
	sprite_t static_animation_sprite;
} assets_global_t;

typedef struct {
	sprite_t office_view_sprite;
	sprite_t fan_animation_sprite;
	sprite_t door_button_sprites[2];
	sprite_t door_animation_sprites[2];
	sprite_t power_usage_sprite;
	sprite_t power_usage_text_sprite;
	sprite_t power_left_sprite;
	sprite_t power_left_percent_sprite;
	sprite_t power_left_number_sprite;
	sprite_t hour_am_sprite;
	sprite_t hour_number_sprite;

	sprite_t camera_view_sprite;
	sprite_t camera_view_name_sprite;
	sprite_t camera_flip_bar_sprite;
	sprite_t camera_flip_animation_sprite;
	sprite_t camera_border_sprite;
	sprite_t camera_map_sprite;
	sprite_t camera_recording_sprite;
	sprite_t camera_button_sprite;
	sprite_t camera_button_name_sprite;
	sprite_t camera_disabled_sprite;

	sound_t fan_sound;
	sound_t light_sound;
	sound_t door_sound;
	sound_t freddy_nose_sound;
	sound_t camera_open_sound;
	sound_t camera_scan_sound;
	sound_t camera_close_sound;
	sound_t camera_blip_sound;

} assets_game_t;

assets_global_t assets_global_create(void);
void assets_global_destroy(assets_global_t *a);

assets_game_t assets_game_create(void);
void assets_game_destroy(assets_game_t *a);

#endif
