/*
plugin.cpp: Copyright (C) 2016  Oka Motofumi

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


#include <algorithm>
#include "myvshelper.h"
#include "ReduceFlicker.h"

static const VSFrameRef* VS_CC
get_frame(int n, int activation_reason, void** instance_data, void**,
          VSFrameContext* frame_ctx, VSCore* core, const VSAPI* api)
{
    auto d = reinterpret_cast<ReduceFlicker*>(*instance_data);

    if (activation_reason == arInitial) {
        d->requestFrames(n, d->vi.numFrames - 1, d->clip, api, frame_ctx);
        return nullptr;
    }
    if (activation_reason != arAllFramesReady) {
        return nullptr;
    }
    return d->getFrame(n, core, api, frame_ctx);
}


static void VS_CC
init_filter(VSMap* in, VSMap* out, void** instance_data, VSNode* node,
            VSCore* core, const VSAPI* api)
{
    auto d = reinterpret_cast<ReduceFlicker*>(*instance_data);
    api->setVideoInfo(&d->vi, 1, node);
}


static void VS_CC
free_filter(void* instance_data, VSCore* core, const VSAPI* api)
{
    auto d = reinterpret_cast<ReduceFlicker*>(instance_data);
    api->freeNode(d->clip);
    delete d;
}


static void
set_planes(int* planes, const VSMap* in, const VSAPI* api)
{
    int num = api->propNumElements(in, "planes");
    validate(num > 3, "length of 'planes' must be equal or smaller than 3.");

    if (num == 0) {
        return;
    }
    for (int i = 0; i < num; ++i) {
        int p = get_arg("planes", planes[i], i, in, api);
        validate(p < 0 || p > 1,
                 "each 'planes' must be set to 0(copy from source) or 1(process).");
        planes[i] = p;
    }
    return;
}



static void VS_CC
create_filter(const VSMap* in, VSMap* out, void*, VSCore* core, const VSAPI* api)
{
    VSNodeRef* clip = api->propGetNode(in, "clip", 0, nullptr);

    try {
        int str = get_arg("strength", 2, 0, in, api);
        validate(str < 1 || str > 3, "strength must be set to 1, 2 or 3.");

        bool agr = get_arg("aggressive", false, 0, in, api);

        int planes[] = {1, 1, 1};
        set_planes(planes, in, api);

        arch_t arch = get_arch(get_arg("opt", 3, 0, in, api));

        auto d = new ReduceFlicker(clip, str, agr, planes, arch, core, api);

        api->createFilter(in, out, "ReduceFlicker", init_filter, get_frame,
                          free_filter, fmParallel, 0, d, core);

    } catch (std::string e) {
        api->freeNode(clip);
        api->setError(out, ("ReduceFlicker: " + e).c_str());
    }
}

VS_EXTERNAL_API(void)
VapourSynthPluginInit(VSConfigPlugin conf, VSRegisterFunction reg, VSPlugin* p)
{
    conf("chikuzen.does.not.have.his.own.domain.reduceflicker", "rdfl",
         "ReduceFlicker for VapourSynth ver. 0.0.0",
         VAPOURSYNTH_API_VERSION, 1, p);
    reg("ReduceFlicker",
        "clip:clip;"
        "strength:int:opt;"
        "aggressive:int:opt;"
        "planes:int[]:opt;"
        "opt:int:opt",
        create_filter, nullptr, p);
}