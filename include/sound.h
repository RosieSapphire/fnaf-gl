#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

typedef uint32_t sound_buffer_t;
typedef uint32_t sound_source_t;
typedef struct {
	sound_buffer_t buffer;
	sound_source_t source;
} sound_t;

void sound_system_create(void);
void sound_system_destroy(void);

sound_buffer_t sound_buffer_create(const char *path);
sound_source_t sound_source_create(sound_buffer_t sound_buffer, const float pitch, const float gain, const float *position, const uint8_t loop);
sound_t sound_create(const char *path, const float pitch, const float gain, const float *position, const uint8_t loop);
void sound_set_gain(const sound_t sound, const float gain);
void sound_play(const sound_t sound);
void sound_stop(const sound_t sound);
void sound_destroy(sound_t *sound);

#endif
