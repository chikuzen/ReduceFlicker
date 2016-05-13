#include <map>
#include <tuple>
#include "ReduceFlicker.h"
#include "myvshelper.h"
#include "proc_filter.h"


template <int STRENGTH>
static void
request_frames(int n, int nf, VSNodeRef* clip, const VSAPI* api,
    VSFrameContext* ctx) noexcept
{
    api->requestFrameFilter(n, clip, ctx);
    api->requestFrameFilter(std::max(n - 1, 0), clip, ctx);
    api->requestFrameFilter(std::max(n - 2, 0), clip, ctx);
    api->requestFrameFilter(std::min(n + 1, nf), clip, ctx);
    if (STRENGTH > 1) {
        api->requestFrameFilter(std::min(n + 2, nf), clip, ctx);
        if (STRENGTH > 2) {
            api->requestFrameFilter(std::max(n - 3, 0), clip, ctx);
            api->requestFrameFilter(std::min(n + 3, nf), clip, ctx);
        }
    }
}


template <int STRENGTH>
static void
recieve_frames(const VSFrameRef** curr, const VSFrameRef** prev,
    const VSFrameRef** next, int n, int nf, VSNodeRef* clip,
    const VSAPI* api, VSFrameContext* ctx) noexcept
{
    *curr = api->getFrameFilter(n, clip, ctx);
    prev[0] = api->getFrameFilter(std::max(n - 1, 0), clip, ctx);
    prev[1] = api->getFrameFilter(std::max(n - 2, 0), clip, ctx);
    next[0] = api->getFrameFilter(std::min(n + 1, nf), clip, ctx);
    if (STRENGTH > 1) {
        next[1] = api->getFrameFilter(std::min(n + 2, nf), clip, ctx);
        if (STRENGTH > 2) {
            prev[2] = api->getFrameFilter(std::max(n - 3, 0), clip, ctx);
            next[2] = api->getFrameFilter(std::min(n + 3, nf), clip, ctx);
        }
    }
}


template <int STRENGTH>
static void
prepare_pointers(const uint8_t** currp, const uint8_t** prevp,
    const uint8_t** nextp, int& cstride, int* pstride,
    int* nstride, const VSFrameRef* curr, const VSFrameRef** prev,
    const VSFrameRef** next, int plane, const VSAPI* api) noexcept
{
    *currp = api->getReadPtr(curr, plane);
    cstride = api->getStride(curr, plane);
    prevp[0] = api->getReadPtr(prev[0], plane);
    pstride[0] = api->getStride(prev[0], plane);
    prevp[1] = api->getReadPtr(prev[1], plane);
    pstride[1] = api->getStride(prev[1], plane);
    nextp[0] = api->getReadPtr(next[0], plane);
    nstride[0] = api->getStride(next[0], plane);
    if (STRENGTH > 1) {
        nextp[1] = api->getReadPtr(next[1], plane);
        nstride[1] = api->getStride(next[1], plane);
        if (STRENGTH > 2) {
            prevp[2] = api->getReadPtr(prev[2], plane);
            pstride[2] = api->getStride(prev[2], plane);
            nextp[2] = api->getReadPtr(next[2], plane);
            nstride[2] = api->getStride(next[2], plane);
        }
    }
}


static proc_filter_t
get_main_proc(arch_t arch, int strength, bool aggressive, int bits_per_sample)
{
    using std::make_tuple;

    std::map<std::tuple<arch_t, int, bool, int>, proc_filter_t> table;

    table[make_tuple(NO_SIMD, 1, false, 8)] =  proc_c<uint8_t, int, 1>;
    table[make_tuple(NO_SIMD, 1, false, 16)] = proc_c<uint16_t, int, 1>;
    table[make_tuple(NO_SIMD, 1, false, 32)] = proc_c<float, float, 1>;

    table[make_tuple(NO_SIMD, 2, false, 8)] =  proc_c<uint8_t, int, 2>;
    table[make_tuple(NO_SIMD, 2, false, 16)] = proc_c<uint16_t, int, 2>;
    table[make_tuple(NO_SIMD, 2, false, 32)] = proc_c<float, float, 2>;

    table[make_tuple(NO_SIMD, 3, false, 8)] =  proc_c<uint8_t, int, 3>;
    table[make_tuple(NO_SIMD, 3, false, 16)] = proc_c<uint16_t, int, 3>;
    table[make_tuple(NO_SIMD, 3, false, 32)] = proc_c<float, float, 3>;

    table[make_tuple(NO_SIMD, 1, true, 8)] =  proc_a_c<uint8_t, int, 1>;
    table[make_tuple(NO_SIMD, 1, true, 16)] = proc_a_c<uint16_t, int, 1>;
    table[make_tuple(NO_SIMD, 1, true, 32)] = proc_a_c<float, float, 1>;

    table[make_tuple(NO_SIMD, 2, true, 8)] =  proc_a_c<uint8_t, int, 2>;
    table[make_tuple(NO_SIMD, 2, true, 16)] = proc_a_c<uint16_t, int, 2>;
    table[make_tuple(NO_SIMD, 2, true, 32)] = proc_a_c<float, float, 2>;

    table[make_tuple(NO_SIMD, 3, true, 8)] =  proc_a_c<uint8_t, int, 3>;
    table[make_tuple(NO_SIMD, 3, true, 16)] = proc_a_c<uint16_t, int, 3>;
    table[make_tuple(NO_SIMD, 3, true, 32)] = proc_a_c<float, float, 3>;

#if defined(__SSE2__)
    table[make_tuple(USE_SSE2, 1, false, 8)]  = proc_simd<uint8_t, __m128i, 1, USE_SSE2>;
    table[make_tuple(USE_SSE2, 1, false, 10)] = proc_simd<int16_t, __m128i, 1, USE_SSE2>;
    table[make_tuple(USE_SSE2, 1, false, 16)] = proc_simd<uint16_t, __m128i, 1, USE_SSE2>;
    table[make_tuple(USE_SSE2, 1, false, 32)] = proc_simd<float, __m128, 1, USE_SSE2>;

    table[make_tuple(USE_SSE2, 2, false, 8)]  = proc_simd<uint8_t, __m128i, 2, USE_SSE2>;
    table[make_tuple(USE_SSE2, 2, false, 10)] = proc_simd<int16_t, __m128i, 2, USE_SSE2>;
    table[make_tuple(USE_SSE2, 2, false, 16)] = proc_simd<uint16_t, __m128i, 2, USE_SSE2>;
    table[make_tuple(USE_SSE2, 2, false, 32)] = proc_simd<float, __m128, 2, USE_SSE2>;

    table[make_tuple(USE_SSE2, 3, false, 8)]  = proc_simd<uint8_t, __m128i, 3, USE_SSE2>;
    table[make_tuple(USE_SSE2, 3, false, 10)] = proc_simd<int16_t, __m128i, 3, USE_SSE2>;
    table[make_tuple(USE_SSE2, 3, false, 16)] = proc_simd<uint16_t, __m128i, 3, USE_SSE2>;
    table[make_tuple(USE_SSE2, 3, false, 32)] = proc_simd<float, __m128, 3, USE_SSE2>;

    table[make_tuple(USE_SSE2, 1, true, 8)]  = proc_a_simd<uint8_t, __m128i, 1, USE_SSE2>;
    table[make_tuple(USE_SSE2, 1, true, 10)] = proc_a_simd<int16_t, __m128i, 1, USE_SSE2>;
    table[make_tuple(USE_SSE2, 1, true, 16)] = proc_a_simd<uint16_t, __m128i, 1, USE_SSE2>;
    table[make_tuple(USE_SSE2, 1, true, 32)] = proc_a_simd<float, __m128, 1, USE_SSE2>;

    table[make_tuple(USE_SSE2, 2, true, 8)]  = proc_a_simd<uint8_t, __m128i, 2, USE_SSE2>;
    table[make_tuple(USE_SSE2, 2, true, 10)] = proc_a_simd<int16_t, __m128i, 2, USE_SSE2>;
    table[make_tuple(USE_SSE2, 2, true, 16)] = proc_a_simd<uint16_t, __m128i, 2, USE_SSE2>;
    table[make_tuple(USE_SSE2, 2, true, 32)] = proc_a_simd<float, __m128, 2, USE_SSE2>;

    table[make_tuple(USE_SSE2, 3, true, 8)]  = proc_a_simd<uint8_t, __m128i, 3, USE_SSE2>;
    table[make_tuple(USE_SSE2, 3, true, 10)] = proc_a_simd<int16_t, __m128i, 3, USE_SSE2>;
    table[make_tuple(USE_SSE2, 3, true, 16)] = proc_a_simd<uint16_t, __m128i, 3, USE_SSE2>;
    table[make_tuple(USE_SSE2, 3, true, 32)] = proc_a_simd<float, __m128, 3, USE_SSE2>;

#if defined(__SSE4_1__)
    table[make_tuple(USE_SSE41, 1, false, 8)]  = proc_simd<uint8_t, __m128i, 1, USE_SSE2>;
    table[make_tuple(USE_SSE41, 1, false, 16)] = proc_simd<uint16_t, __m128i, 1, USE_SSE41>;
    table[make_tuple(USE_SSE41, 1, false, 32)] = proc_simd<float, __m128, 1, USE_SSE2>;

    table[make_tuple(USE_SSE41, 2, false, 8)]  = proc_simd<uint8_t, __m128i, 2, USE_SSE2>;
    table[make_tuple(USE_SSE41, 2, false, 16)] = proc_simd<uint16_t, __m128i, 2, USE_SSE41>;
    table[make_tuple(USE_SSE41, 2, false, 32)] = proc_simd<float, __m128, 2, USE_SSE2>;

    table[make_tuple(USE_SSE41, 3, false, 8)]  = proc_simd<uint8_t, __m128i, 3, USE_SSE2>;
    table[make_tuple(USE_SSE41, 3, false, 16)] = proc_simd<uint16_t, __m128i, 3, USE_SSE41>;
    table[make_tuple(USE_SSE41, 3, false, 32)] = proc_simd<float, __m128, 3, USE_SSE2>;

    table[make_tuple(USE_SSE41, 1, true, 8)]  = proc_a_simd<uint8_t, __m128i, 1, USE_SSE41>;
    table[make_tuple(USE_SSE41, 1, true, 16)] = proc_a_simd<uint16_t, __m128i, 1, USE_SSE41>;
    table[make_tuple(USE_SSE41, 1, true, 32)] = proc_a_simd<float, __m128, 1, USE_SSE41>;

    table[make_tuple(USE_SSE41, 2, true, 8)]  = proc_a_simd<uint8_t, __m128i, 2, USE_SSE41>;
    table[make_tuple(USE_SSE41, 2, true, 16)] = proc_a_simd<uint16_t, __m128i, 2, USE_SSE41>;
    table[make_tuple(USE_SSE41, 2, true, 32)] = proc_a_simd<float, __m128, 2, USE_SSE41>;

    table[make_tuple(USE_SSE41, 3, true, 8)]  = proc_a_simd<uint8_t, __m128i, 3, USE_SSE41>;
    table[make_tuple(USE_SSE41, 3, true, 16)] = proc_a_simd<uint16_t, __m128i, 3, USE_SSE41>;
    table[make_tuple(USE_SSE41, 3, true, 32)] = proc_a_simd<float, __m128, 3, USE_SSE41>;

#if defined(__AVX2__)
    table[make_tuple(USE_AVX2, 1, false, 8)]  = proc_simd<uint8_t, __m256i, 1, USE_AVX2>;
    table[make_tuple(USE_AVX2, 1, false, 16)] = proc_simd<uint16_t, __m256i, 1, USE_AVX2>;
    table[make_tuple(USE_AVX2, 1, false, 32)] = proc_simd<float, __m128, 1, USE_AVX2>;

    table[make_tuple(USE_AVX2, 2, false, 8)]  = proc_simd<uint8_t, __m256i, 2, USE_AVX2>;
    table[make_tuple(USE_AVX2, 2, false, 16)] = proc_simd<uint16_t, __m256i, 2, USE_AVX2>;
    table[make_tuple(USE_AVX2, 2, false, 32)] = proc_simd<float, __m128, 2, USE_AVX2>;

    table[make_tuple(USE_AVX2, 3, false, 8)]  = proc_simd<uint8_t, __m256i, 3, USE_AVX2>;
    table[make_tuple(USE_AVX2, 3, false, 16)] = proc_simd<uint16_t, __m256i, 3, USE_AVX2>;
    table[make_tuple(USE_AVX2, 3, false, 32)] = proc_simd<float, __m256, 3, USE_AVX2>;

    table[make_tuple(USE_AVX2, 1, true, 8)]  = proc_a_simd<uint8_t, __m256i, 1, USE_AVX2>;
    table[make_tuple(USE_AVX2, 1, true, 16)] = proc_a_simd<uint16_t, __m256i, 1, USE_AVX2>;
    table[make_tuple(USE_AVX2, 1, true, 32)] = proc_a_simd<float, __m256, 1, USE_AVX2>;

    table[make_tuple(USE_AVX2, 2, true, 8)]  = proc_a_simd<uint8_t, __m256i, 2, USE_AVX2>;
    table[make_tuple(USE_AVX2, 2, true, 16)] = proc_a_simd<uint16_t, __m256i, 2, USE_AVX2>;
    table[make_tuple(USE_AVX2, 2, true, 32)] = proc_a_simd<float, __m256, 2, USE_AVX2>;

    table[make_tuple(USE_AVX2, 3, true, 8)]  = proc_a_simd<uint8_t, __m256i, 3, USE_AVX2>;
    table[make_tuple(USE_AVX2, 3, true, 16)] = proc_a_simd<uint16_t, __m256i, 3, USE_AVX2>;
    table[make_tuple(USE_AVX2, 3, true, 32)] = proc_a_simd<float, __m256, 3, USE_AVX2>;
#endif
#endif
#endif

    return table[make_tuple(arch, strength, aggressive, bits_per_sample)];

}

ReduceFlicker::
ReduceFlicker(VSNodeRef* c, int s, bool aggressive, int* planes, arch_t arch,
              VSCore* core, const VSAPI* api) :
    strength(s), clip(c)
{
    vi = *api->getVideoInfo(clip);
    validate(!is_constant_format(vi), "clip is not constant format.");
    validate(is_half_precision(vi), "half precision is not supported.");
    
    memcpy(procType, planes, sizeof(int) * 3);

    switch (strength) {
    case 1:
        requestFrames = request_frames<1>;
        recieveFrames = recieve_frames<1>;
        prepareSrcPtrs = prepare_pointers<1>;
        break;
    case 2:
        requestFrames = request_frames<2>;
        recieveFrames = recieve_frames<2>;
        prepareSrcPtrs = prepare_pointers<2>;
        break;
    default:
        requestFrames = request_frames<3>;
        recieveFrames = recieve_frames<3>;
        prepareSrcPtrs = prepare_pointers<3>;
    }

    int bps = vi.format->bitsPerSample == 9 ? 10 : vi.format->bitsPerSample;
    if (arch != USE_SSE2 && bps == 10) {
        bps = 16;
    }

    mainProc = get_main_proc(arch, strength, aggressive, bps);

}



const VSFrameRef* ReduceFlicker::
getFrame(int n, VSCore* core, const VSAPI* api, VSFrameContext* ctx)
{
    const int nf = vi.numFrames - 1;
    const VSFrameRef *curr = nullptr, *prev[3], *next[3];

    recieveFrames(&curr, prev, next, n, nf, clip, api, ctx);

    const VSFormat* fmt = api->getFrameFormat(curr);

    VSFrameRef* dst = api->newVideoFrame(fmt, vi.width, vi.height, curr, core);

    for (int p = 0; p < fmt->numPlanes; ++p) {
        if (procType[p] == 0) {
            bitblt(api->getWritePtr(dst, p), api->getStride(dst, p),
                   api->getReadPtr(curr, p), api->getStride(curr, p),
                   get_row_size(dst, p, fmt, api), api->getFrameHeight(dst, p));
            continue;
        }

        int cstride, pstride[3], nstride[3];
        const uint8_t *currp, *prevp[3], *nextp[3];
        prepareSrcPtrs(&currp, prevp, nextp, cstride, pstride, nstride, curr,
                       prev, next, p, api);

        uint8_t* dstp = api->getWritePtr(dst, p);
        int dstride = api->getStride(dst, p);
        size_t width = api->getFrameWidth(dst, p);
        size_t height = api->getFrameHeight(dst, p);

        mainProc(dstp, currp, prevp, nextp, dstride, cstride, pstride, nstride,
                 width, height);
    }

    api->freeFrame(curr);
    api->freeFrame(prev[0]);
    api->freeFrame(prev[1]);
    api->freeFrame(next[0]);
    if (strength > 1) {
        api->freeFrame(next[1]);
    }
    if (strength > 2) {
        api->freeFrame(prev[2]);
        api->freeFrame(next[2]);
    }

    return dst;
}
