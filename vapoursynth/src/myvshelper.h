/*
myvshelper.h: Copyright (C) 2016  Oka Motofumi

Author: Oka Motofumi (chikuzen.mo at gmail dot com)

This file is part of ReduceFlicker.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with the author; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


#ifndef MY_VAPOURSYNTH_HELPER_H
#define MY_VAPOURSYNTH_HELPER_H

#include <cstdint>
#include <cstdlib>
#include <string>
#include <VapourSynth.h>
#include "arch.h"


static F_INLINE void validate(bool cond, const char* msg)
{
    if (cond) {
        throw std::string(msg);
    }
}

static F_INLINE bool is_constant_format(const VSVideoInfo& vi)
{
    return vi.height > 0 && vi.width > 0 && vi.format;
}

static F_INLINE bool is_half_precision(const VSVideoInfo& vi)
{
    return vi.format->sampleType == stFloat && vi.format->bitsPerSample == 16;
}


template <typename T>
static F_INLINE T get_prop(const VSAPI*, const VSMap*, const char*, int, int*);

template <>
F_INLINE int32_t
get_prop<int32_t>(const VSAPI* api, const VSMap* in, const char* name, int idx,
                  int* e)
{
    return static_cast<int32_t>(api->propGetInt(in, name, idx, e));
}

template <>
F_INLINE int64_t
get_prop<int64_t>(const VSAPI* api, const VSMap* in, const char* name, int idx,
                  int* e)
{
    return api->propGetInt(in, name, idx, e);
}

template <>
F_INLINE bool
get_prop<bool>(const VSAPI* api, const VSMap* in, const char* name, int idx,
               int* e)
{
    return api->propGetInt(in, name, idx, e) != 0;
}

template <>
F_INLINE float
get_prop<float>(const VSAPI* api, const VSMap* in, const char* name, int idx,
                int* e)
{
    return static_cast<float>(api->propGetFloat(in, name, idx, e));
}

template <>
F_INLINE double
get_prop<double>(const VSAPI* api, const VSMap* in, const char* name, int idx,
                 int* e)
{
    return api->propGetFloat(in, name, idx, e);
}

template <>
F_INLINE const char*
get_prop<const char*>(const VSAPI* api, const VSMap* in, const char* name,
                      int idx, int* e)
{
    return api->propGetData(in, name, idx, e);
}

template <typename T>
static inline T
get_arg(const char* name, const T default_value, int index, const VSMap* in,
        const VSAPI* api)
{
    int err = 0;
    T ret = get_prop<T>(api, in, name, index, &err);
    if (err) {
        ret = default_value;
    }
    return ret;
}

static F_INLINE int
get_sized_stride(const VSFrameRef* f, const int plane, const VSVideoInfo& vi,
                 const VSAPI* api)
{
    return api->getStride(f, plane) / vi.format->bytesPerSample;
}


static F_INLINE size_t
get_row_size(const VSFrameRef* f, const int plane, const VSFormat* fmt,
             const VSAPI* api)
{
    return api->getFrameWidth(f, plane) * fmt->bytesPerSample;
}

template <typename T>
static F_INLINE T my_malloc(size_t size, size_t align=16)
{
#if defined(__SSE2__)
    void* ptr = _mm_malloc(size, align);
#else
    void* ptr = malloc(size);
#endif
    validate(!ptr, "failed to allocate memory.");
    return static_cast<T>(ptr);
}


static F_INLINE void my_free(void* ptr)
{
#if defined(__SSE2__)
     _mm_free(ptr);
#else
    if (ptr) free(ptr);
#endif
     ptr = nullptr;
}


template <typename T>
static inline void
bitblt(T* dstp, const int dstride, const T* srcp, const int sstride,
       const size_t row_size, const size_t height)
{
    if (height == 0) {
        return;
    }
    if (sstride == dstride && sstride == static_cast<int>(row_size)) {
        memcpy(dstp, srcp, row_size * height);
        return;
    }
    for (size_t y = 0; y < height; ++y) {
        memcpy(dstp, srcp, row_size);
        srcp += sstride;
        dstp += dstride;
    }
}


#endif
