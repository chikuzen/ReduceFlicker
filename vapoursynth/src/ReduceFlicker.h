/*
ReduceFlicker.h: Copyright (C) 2016  Oka Motofumi

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


#ifndef VS_REDUCE_FLICKER_H
#define VS_REDUCE_FLICKER_H

#include <VapourSynth.h>
#include "arch.h"


typedef void (*proc_filter_t)(
    uint8_t* dstp, const uint8_t* currp, const uint8_t** prevp,
    const uint8_t** nextp, int dstride, int cstride, int* pstride, int* nstride,
    size_t width, size_t height);


class ReduceFlicker {
    int strength;
    int procType[3];

    void(*recieveFrames)(
        const VSFrameRef** curr, const VSFrameRef** prev,
        const VSFrameRef** next, int n, int nf, VSNodeRef* clip,
        const VSAPI* api, VSFrameContext* ctx);
    void(*prepareSrcPtrs)(
        const uint8_t** currp, const uint8_t** prevp, const uint8_t** nextp,
        int& cstride, int* pstride, int* nstride, const VSFrameRef* curr,
        const VSFrameRef** prev, const VSFrameRef** next, int plane,
        const VSAPI* api);
    proc_filter_t mainProc;

public:
    VSNodeRef* clip;
    VSVideoInfo vi;

    void(*requestFrames)(
        int n, int nf, VSNodeRef* clip, const VSAPI* api, VSFrameContext* ctx);

    ReduceFlicker(VSNodeRef* clip, int strength, bool aggressive, int* planes,
                  arch_t arch, VSCore* core, const VSAPI* api);
    ~ReduceFlicker() {}
    const VSFrameRef* getFrame(int n, VSCore* core, const VSAPI* api,
                               VSFrameContext* ctx);
};

#endif
