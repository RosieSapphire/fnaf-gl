#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

uint32_t sound_buffer_create(const char *path);
uint32_t sound_source_create(uint32_t sound_buffer, const float pitch, const float gain, const float *position, const uint8_t loop);

#endif
