#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>

typedef uint32_t texture_t;
texture_t texture_create(const char *path, const int32_t wrap_mode, const int32_t min_interpolation, const int32_t mag_interpolation);

#endif
