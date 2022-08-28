#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

typedef uint32_t sound_buffer_t;
typedef uint32_t sound_source_t;
sound_buffer_t sound_buffer_create(const char *path);
sound_source_t sound_source_create(sound_buffer_t sound_buffer, const float pitch, const float gain, const float *position, const uint8_t loop);

#endif
