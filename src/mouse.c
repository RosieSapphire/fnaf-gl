#include "mouse.h"

void mouse_get_position(GLFWwindow *window, ivec2 output) {
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	output[0] = (int32_t)x;
	output[1] = (int32_t)y;
}

uint8_t mouse_inside_box(GLFWwindow *window, const ivec4 box, const float offset) {
	ivec2 mouse_position;
	mouse_get_position(window, mouse_position);
	return
		mouse_position[0] > box[0] + offset &&
		mouse_position[0] < box[1] + offset &&
		mouse_position[1] > box[2]			&&
		mouse_position[1] < box[3];
}
