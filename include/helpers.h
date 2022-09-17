#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

float clampf(const float x, const float min, const float max);

/* Just a more optimized "fmod" type function */
float fmod2(const float val, const float mod);

/* Converting Clickteam Fusion's animation speed to tick */
float blink_timer_get_tick(const float time_now, const float animation_percent, const float animation_framerate, const float mod_max);

/* Getting the mouse pos relative to a window */
void mouse_get_position(GLFWwindow *window, ivec2 output);

/* Getting the mouse pos relative to a window */
uint8_t mouse_inside_box(const ivec2 mouse_pos, const ivec4 box, const int32_t offset);

#endif
