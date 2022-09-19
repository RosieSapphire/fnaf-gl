#include "helpers.h"

#include <stdint.h>

float clampf(const float x, const float min, const float max) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	#pragma GCC diagnostic ignored "-Wuninitialized"
    float diff[2] = {min-x, x-max};
    uint32_t MemA = *(uint32_t*)&diff[0];
    uint32_t MemB = *(uint32_t*)&diff[1];
    uint8_t Flag_A = (MemA>>31);
    uint8_t Flag_B = (MemB>>31);
    uint8_t or = (Flag_A & Flag_B);
	#pragma GCC diagnostic pop
    return (or * x) + ((1-Flag_A) * min) + ((1-Flag_B) * max);
}

float fmod2(const float val, const float mod) {
	const float val_scaled = val / mod;
	return (val_scaled - (float)((int32_t)val_scaled)) * mod;
}

float blink_timer_get_tick(const float time_now, const float animation_percent, const float animation_framerate, const float mod_max) {
	return fmod2(time_now * ((animation_percent / (100.0f / animation_framerate)) / 2.0f), mod_max);
}

void mouse_get_position(GLFWwindow *window, ivec2 output) {
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	output[0] = (int32_t)x;
	output[1] = (int32_t)y;
}

uint8_t mouse_inside_box(const ivec2 mouse_pos, const ivec4 box, const int32_t offset) {
	return
		mouse_pos[0] > box[0] + offset &&
		mouse_pos[0] < box[0] + box[2] + offset &&
		mouse_pos[1] > box[1] &&
		mouse_pos[1] < box[1] + box[3];
}
