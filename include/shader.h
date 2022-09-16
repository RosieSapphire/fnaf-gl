#ifndef SHADER_H
#define SHADER_H

#include <stdint.h>

typedef uint32_t shader_t;
shader_t shader_create(const char *shader_vertex_path, const char *shader_fragment_path);

#endif
