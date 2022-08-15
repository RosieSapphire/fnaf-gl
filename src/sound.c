#include "sound.h"

#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <sndfile.h>
#include <limits.h>

uint32_t sound_buffer_create(const char *path) {
	uint32_t sound_buffer;

	SNDFILE *file;
	SF_INFO file_info;
	sf_count_t frame_count;
	uint32_t error;
	uint32_t format;
	int16_t *memory_buffer;
	uint32_t size;

	uint32_t formats[4] = {
		AL_FORMAT_MONO16,
		AL_FORMAT_STEREO16,
		AL_FORMAT_BFORMAT2D_16,
		AL_FORMAT_BFORMAT3D_16
	};

	file = sf_open(path, SFM_READ, &file_info);
	if(!file) {
		printf("ERROR: Sound loading fucked up: %s", path);
		return 0;
	}

	if(file_info.frames < 1 || file_info.frames > (sf_count_t)(INT_MAX / sizeof(int16_t)) / file_info.channels) {
		sf_close(file);
		printf("ERROR: Sample rate fucked up: %ld.", file_info.frames);
		return 0;
	}

	format = AL_NONE;
	format = formats[file_info.channels - 1];
	if(!format) {
		sf_close(file);
		printf("ERROR: Channel count fucked up.");
		return 0;
	}

	memory_buffer = malloc((size_t)(file_info.frames * file_info.channels) * sizeof(int16_t));
	frame_count = sf_readf_short(file, memory_buffer, file_info.frames);
	if(frame_count < 1) {
		free(memory_buffer);
		sf_close(file);
		printf("ERROR: Sample reading fucked up.");
		return 0;
	}

	size = (size_t)(frame_count * file_info.channels) * (size_t)sizeof(int16_t);

	sound_buffer = 0;
	alGenBuffers(1, &sound_buffer);
	alBufferData(sound_buffer, format, (void *)memory_buffer, size, file_info.samplerate);
	error = alGetError();

	free(memory_buffer);
	sf_close(file);

	if(error) {
		if(sound_buffer && alIsBuffer(sound_buffer)) {
			alDeleteBuffers(1, &sound_buffer);
		}
		printf("ERROR: Something fucked up: %s\n", alGetString(error));
		return 0;
	}

	return sound_buffer;
}

uint32_t sound_source_create(uint32_t sound_buffer, const float pitch, const float gain, const float *position, const uint8_t loop) {
	uint32_t sound_source;
	alGenSources(1, &sound_source); alSourcef(sound_source, AL_PITCH, pitch);
	alSourcef(sound_source, AL_GAIN, gain);
	alSourcefv(sound_source, AL_POSITION, position);
	alSource3f(sound_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alSourcei(sound_source, AL_LOOPING, loop);
	alSourcei(sound_source, AL_BUFFER, sound_buffer);

	return sound_source;
}
