#include "file.h"

#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

char *file_load_contents(const char *path) {
	uint32_t size;
	FILE *file;
	char *buffer;

	file = fopen(path, "rb");
	#ifdef DEBUG
   		if(!file) {
   		    printf("ERROR: File loading fucked up.\n");
   		    return NULL;
   		}
	#endif

    fseek(file, 0L, SEEK_END);
    size = (uint32_t)ftell(file);
    rewind(file);

    buffer = malloc(size * sizeof(GLchar));
	fread(buffer, sizeof(uint8_t), size, file);
    buffer[size - 1] = '\0';

    fclose(file);

	return buffer;
}
