#include "file.h"

#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

char *file_load_contents(const char *path) {
	uint32_t i;
	uint32_t size;
	FILE *file;
	char *buffer;

	file = fopen(path, "rb");
    if(!file) {
        printf("ERROR: File loading fucked up.\n");
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size = (uint32_t)ftell(file);
    rewind(file);

    buffer = malloc(size * sizeof(GLchar));
    for(i = 0; i < size; i++) {
        buffer[i] = (char)fgetc(file);
    }
    buffer[size - 1] = '\0';

    fclose(file);

	return buffer;
}
