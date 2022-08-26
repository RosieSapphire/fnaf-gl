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
	uint64_t frame_count;
	int32_t format;
	int16_t *memory_buffer;
	uint64_t size;

	#ifdef DEBUG
		int32_t error;
	#endif

	int32_t formats[4] = {
		AL_FORMAT_MONO16,
		AL_FORMAT_STEREO16,
		AL_FORMAT_BFORMAT2D_16,
		AL_FORMAT_BFORMAT3D_16
	};

	file = sf_open(path, SFM_READ, &file_info);
	format = AL_NONE;
	format = formats[file_info.channels - 1];
	#ifdef DEBUG
		if(!file) {
			printf("ERROR: Sound loading fucked up: %s", path);
			return 0;
		}

		if(file_info.frames < 1 || file_info.frames > (sf_count_t)(INT_MAX / sizeof(int16_t)) / file_info.channels) {
			sf_close(file);
			printf("ERROR: Sample rate fucked up: %ld.", file_info.frames);
			return 0;
		}

		if(!format) {
			sf_close(file);
			printf("ERROR: Channel count fucked up.");
			return 0;
		}
	#endif

	memory_buffer = malloc((uint64_t)(file_info.frames * file_info.channels) * sizeof(int16_t));
	frame_count = (uint64_t)sf_readf_short(file, memory_buffer, file_info.frames);
	#ifdef DEBUG
		if(frame_count < 1) {
			free(memory_buffer);
			sf_close(file);
			printf("ERROR: Sample reading fucked up.");
			return 0;
		}
	#endif

	size = frame_count * (uint64_t)file_info.channels * sizeof(int16_t);

	sound_buffer = 0;
	alGenBuffers(1, &sound_buffer);
	alBufferData(sound_buffer, format, (void *)memory_buffer, (int32_t)size, file_info.samplerate);

	#ifdef DEBUG
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
	#endif

	return sound_buffer;
}

uint32_t sound_source_create(int32_t sound_buffer, const float pitch, const float gain, const float *position, const uint8_t loop) {
	uint32_t sound_source;
	alGenSources(1, &sound_source); alSourcef(sound_source, AL_PITCH, pitch);
	alSourcef(sound_source, AL_GAIN, gain);
	alSourcefv(sound_source, AL_POSITION, position);
	alSource3f(sound_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alSourcei(sound_source, AL_LOOPING, loop);
	alSourcei(sound_source, AL_BUFFER, sound_buffer);

	return sound_source;
}
