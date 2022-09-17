#include "sound.h"

#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <sndfile.h>
#include <limits.h>

#include <assert.h>

static ALCdevice *sound_device;
static ALCcontext *sound_context;

void sound_system_create() {
	#ifdef DEBUG
		const char *sound_device_name;
	#endif
	sound_device = alcOpenDevice(NULL);
	#ifdef DEBUG
		if(!sound_device) {
		    printf("ERROR: Audio Device fucked up.");
			assert(0);
		}
	#endif
	
	sound_context = alcCreateContext(sound_device, NULL);
	#ifdef DEBUG
		if(!sound_context) {
		    printf("ERROR: Audio Context fucked up.");
			assert(0);
		}
	#endif
	
	#ifdef DEBUG
	if(!alcMakeContextCurrent(sound_context)) {
	    printf("ERROR: Making context fucked up.");
		assert(0);
	}
	#else
		alcMakeContextCurrent(sound_context);
	#endif
	
	#ifdef DEBUG
		sound_device_name = NULL;
		if(alcIsExtensionPresent(sound_device, "ALC_ENUMERATE_ALL_EXT")) {
		    sound_device_name = alcGetString(sound_device, ALC_ALL_DEVICES_SPECIFIER);
		}
		
		if(!sound_device_name || alcGetError(sound_device) != AL_NO_ERROR) {
		    sound_device_name = alcGetString(sound_device, ALC_DEVICE_SPECIFIER);
		}
		printf("SOUND DEVICE: %s\n", sound_device_name);
	#endif
}

void sound_system_destroy() {
	alcDestroyContext(sound_context);
	alcCloseDevice(sound_device);
}

sound_buffer_t sound_buffer_create(const char *path) {
	sound_buffer_t sound_buffer;
	SNDFILE *file;
	SF_INFO file_info;
	uint64_t frame_count;
	int32_t format = AL_NONE;
	int16_t *buffer;
	uint64_t size;

	/*
	#ifdef DEBUG
		int32_t error;
	#endif
	*/

	int32_t formats[2] = {
		AL_FORMAT_MONO16,
		AL_FORMAT_STEREO16,
	};

	file = sf_open(path, SFM_READ, &file_info);
	format = formats[file_info.channels - 1];
	#ifdef DEBUG
		if(!file) {
			printf("ERROR: Sound loading fucked up: %s", path);
			assert(0);
		}

		if(file_info.frames < 1 || file_info.frames > (sf_count_t)(INT_MAX / sizeof(int16_t)) / file_info.channels) {
			sf_close(file);
			printf("ERROR: Sample rate fucked up: %ld.", file_info.frames);
			assert(0);
		}

		if((format != AL_FORMAT_MONO16) && (format != AL_FORMAT_STEREO16)) {
			sf_close(file);
			printf("ERROR: Format fucked up at path '%s': %u\n", path, format);
			assert(0);
		}
	#endif

	buffer = malloc((uint64_t)(file_info.frames * file_info.channels) * sizeof(int16_t));
	frame_count = (uint64_t)sf_readf_short(file, buffer, file_info.frames);
	#ifdef DEBUG
		if(frame_count < 1) {
			free(buffer);
			sf_close(file);
			printf("ERROR: Sample reading fucked up.");
			assert(0);
		}
	#endif

	size = frame_count * (uint64_t)file_info.channels * sizeof(int16_t);

	sound_buffer = 0;
	alGenBuffers(1, &sound_buffer);
	alBufferData(sound_buffer, format, (void *)buffer, (int32_t)size, file_info.samplerate);

	/*
	#ifdef DEBUG
		error = alGetError();
		free(buffer);
		sf_close(file);

		if(error) {
			printf("ERROR: Buffers fucked up at path '%s': %s\n", path, alGetString(error));
			assert(0);
		}
	#endif
	*/

	return sound_buffer;
}

sound_source_t sound_source_create(sound_buffer_t sound_buffer, const float pitch, const float gain, const float *position, const uint8_t loop) {
	uint32_t sound_source;
	alGenSources(1, &sound_source); alSourcef(sound_source, AL_PITCH, pitch);
	alSourcef(sound_source, AL_GAIN, gain);
	alSourcefv(sound_source, AL_POSITION, position);
	alSource3f(sound_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alSourcei(sound_source, AL_LOOPING, loop);
	alSourcei(sound_source, AL_BUFFER, (int32_t)sound_buffer);

	return sound_source;
}

sound_t sound_create(const char *path, const float pitch, const float gain, const float *position, const uint8_t loop) {
	sound_t sound;
	sound.buffer = sound_buffer_create(path);
	sound.source = sound_source_create(sound.buffer, pitch, gain, position, loop);
	return sound;
}

void sound_set_gain(const sound_t sound, const float gain) {
	alSourcef(sound.source, AL_GAIN, gain);
}

void sound_play(const sound_t sound) {
	alSourcePlay(sound.source);
}

void sound_stop(const sound_t sound) {
	alSourceStop(sound.source);
}

void sound_destroy(sound_t *sound) {
	alDeleteSources(1, &sound->source);
	alDeleteBuffers(1, &sound->buffer);
}
