#ifndef REDUCE_FLICKER_H
#define REDUCE_FLICKER_H

#include <cstdint>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <avisynth.h>

#define REDUCE_FLICKER_VERSION "0.0.1"


enum arch_t {
    NO_SIMD = 0,
    USE_SSE2 = 1,
    USE_AVX2
};


typedef IScriptEnvironment ise_t;


typedef void(__stdcall *proc_filter_t)(
    uint8_t* dstp, const uint8_t** prevp, const uint8_t* currp,
    const uint8_t** nextp, const int dpitch, const int* ppitch,
    const int cpitch, const int* npitch, const int width, const int height);


class ReduceFlicker : public GenericVideoFilter {
    int numPlanes;
    const int strength;
    size_t align;
    bool raccess;

    proc_filter_t mainProc;

public:
    ReduceFlicker(PClip c, int str, bool agr, bool grey, arch_t arch, bool raccess);
    ~ReduceFlicker() {}
    PVideoFrame __stdcall GetFrame(int n, ise_t* env);
    static AVSValue __cdecl create(AVSValue args, void*, ise_t* env);
};



#endif
