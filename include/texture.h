#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>

uint32_t texture_create(const char *path, const int32_t wrap_mode, const int32_t min_interpolation, const int32_t mag_interpolation);
void texture_destroy(uint32_t *texture);

#endif
