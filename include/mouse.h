#ifndef MOUSE_H
#define MOUSE_H

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

void mouse_get_position(GLFWwindow *window, ivec2 output);
uint8_t mouse_inside_box(GLFWwindow *window, const ivec4 box, const int32_t offset);

#endif
