#include "helpers.h"

#include <stdint.h>

float clampf(const float x, const float min, const float max) {
    float diff[2] = {min-x, x-max};
    uint32_t MemA = *(uint32_t*)&diff[0];
    uint32_t MemB = *(uint32_t*)&diff[1];
    uint8_t Flag_A = (MemA>>31);
    uint8_t Flag_B = (MemB>>31);
    uint8_t OOR = (Flag_A & Flag_B);
    return (OOR * x) + ((1-Flag_A) * min) + ((1-Flag_B) * max);
}

float fmod2(const float val, const float mod) {
	const float val_scaled = val / mod;
	return (val_scaled - (float)((int32_t)val_scaled)) * mod;
}
