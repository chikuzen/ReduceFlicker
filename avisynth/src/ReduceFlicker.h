#ifndef REDUCE_FLICKER_H
#define REDUCE_FLICKER_H

#include <cstdint>

enum arch_t {
    NO_SIMD = 0,
    USE_SSE2 = 1,
    USE_AVX2
};

typedef void(__stdcall *proc_filter_t)(
    uint8_t* dstp, const uint8_t** prevp, const uint8_t* currp,
    const uint8_t** nextp, const int dpitch, const int* ppitch,
    const int cpitch, const int* npitch, const int width, const int height);

proc_filter_t get_main_proc(int strength, bool aggressive, arch_t arch);



#endif
