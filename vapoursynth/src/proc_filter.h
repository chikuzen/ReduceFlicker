/*
proc_filter.h: Copyright (C) 2016  Oka Motofumi

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


#ifndef REDUCE_FLICKER_PROC_FILTER_H
#define REDUCE_FLICKER_PROC_FILTER_H

#include <stdint.h>
#include <algorithm>
#include "arch.h"


template <typename T>
static F_INLINE T absdiff(const T x, const T y)
{
    return x < y ? y - x : x - y;
}

template <typename T>
static F_INLINE T clamp(T x, T minimum, T maximum)
{
    return std::min(std::max(x, minimum), maximum);
}

template <typename T>
static F_INLINE T get_avg(T a, T b, T x)
{
    int t = std::max((a + b + 1) / 2 - 1, 0);
    return static_cast<T>((t + x + 1) / 2);
}

template <>
F_INLINE float get_avg(float a, float b, float x)
{
    return (a + b + x + x) * 0.25f;
}


template <typename T0, typename T1, int STRENGTH>
static void
proc_c(uint8_t* dstp, const uint8_t* currp, const uint8_t** prevp,
       const uint8_t** nextp, int dstride, int cstride, int* pstride,
       int* nstride, size_t width, size_t height) noexcept
{
    const T0 *prv0, *prv1, *prv2, *nxt0, *nxt1, *nxt2;

    T0 *dst0 = reinterpret_cast<T0*>(dstp);
    const T0* cur0 = reinterpret_cast<const T0*>(currp);
    prv0 = reinterpret_cast<const T0*>(prevp[0]);
    prv1 = reinterpret_cast<const T0*>(prevp[1]);
    nxt0 = reinterpret_cast<const T0*>(prevp[0]);
    dstride /= sizeof(T0);
    cstride /= sizeof(T0);
    pstride[0] /= sizeof(T0);
    pstride[1] /= sizeof(T0);
    nstride[0] /= sizeof(T0);
    if (STRENGTH > 1) {
        nxt1 = reinterpret_cast<const T0*>(nextp[1]);
        nstride[1] /= sizeof(T0);
    }
    if (STRENGTH > 2) {
        prv2 = reinterpret_cast<const T0*>(prevp[2]);
        nxt2 = reinterpret_cast<const T0*>(prevp[2]);
        pstride[2] /= sizeof(T0);
        nstride[2] /= sizeof(T0);
    }

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            const T1 curx = static_cast<T1>(cur0[x]);
            T1 d = absdiff(curx, static_cast<T1>(prv1[x]));
            if (STRENGTH > 1) {
                d = std::min(d, absdiff(curx, static_cast<T1>(nxt1[x])));
            }
            if (STRENGTH > 2) {
                d = std::min(d, absdiff(curx, static_cast<T1>(prv2[x])));
                d = std::min(d, absdiff(curx, static_cast<T1>(nxt2[x])));
            }
            T1 prvx = static_cast<T1>(prv0[x]);
            T1 nxtx = static_cast<T1>(nxt0[x]);
            T1 avg = get_avg(prvx, nxtx, curx);
            T1 ul = std::max(std::min(prvx, nxtx) - d, curx);
            T1 ll = std::min(std::max(prvx, nxtx) + d, curx);
            dst0[x] = static_cast<T0>(clamp(avg, ll, ul));
        }
        dst0 += dstride;
        cur0 += cstride;
        prv0 += pstride[0];
        prv1 += pstride[1];
        nxt0 += nstride[0];
        if (STRENGTH > 1) {
            nxt1 += nstride[1];
        }
        if (STRENGTH > 2) {
            prv2 += pstride[2];
            nxt2 += nstride[2];
        }
    }
}


template <typename T>
static F_INLINE void update_diff(T x, T y, T& d1, T& d2)
{
    T d = x - y;
    if (d >= 0) {
        d2 = 0;
        d1 = std::min(d, d1);
    } else {
        d1 = 0;
        d2 = std::min(-d, d2);
    }
}


template <typename T0, typename T1, int STRENGTH>
static void
proc_a_c(uint8_t* dstp, const uint8_t* currp, const uint8_t** prevp,
         const uint8_t** nextp, int dstride, int cstride, int* pstride,
         int* nstride, size_t width, size_t height) noexcept
{
    const T0 *prv0, *prv1, *prv2, *nxt0, *nxt1, *nxt2;

    T0 *dst0 = reinterpret_cast<T0*>(dstp);
    const T0* cur0 = reinterpret_cast<const T0*>(currp);
    prv0 = reinterpret_cast<const T0*>(prevp[0]);
    prv1 = reinterpret_cast<const T0*>(prevp[1]);
    nxt0 = reinterpret_cast<const T0*>(prevp[0]);
    dstride /= sizeof(T0);
    cstride /= sizeof(T0);
    pstride[0] /= sizeof(T0);
    pstride[1] /= sizeof(T0);
    nstride[0] /= sizeof(T0);
    if (STRENGTH > 1) {
        nxt1 = reinterpret_cast<const T0*>(nextp[1]);
        nstride[1] /= sizeof(T0);
    }
    if (STRENGTH > 2) {
        prv2 = reinterpret_cast<const T0*>(prevp[2]);
        nxt2 = reinterpret_cast<const T0*>(prevp[2]);
        pstride[2] /= sizeof(T0);
        nstride[2] /= sizeof(T0);
    }

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            const T1 curx = static_cast<T1>(cur0[x]);
            T1 d1 = prv1[x] - curx;
            T1 d2 = 0;
            if (d1 < 0) {
                d2 = -d1;
                d1 = 0;
            }
            if (STRENGTH > 1) {
                update_diff(static_cast<T1>(nxt1[x]), curx, d1, d2);
            }
            if (STRENGTH > 2) {
                update_diff(static_cast<T1>(prv2[x]), curx, d1, d2);
                update_diff(static_cast<T1>(nxt2[x]), curx, d1, d2);
            }
            T1 prvx = static_cast<T1>(prv0[x]);
            T1 nxtx = static_cast<T1>(nxt0[x]);
            T1 avg = get_avg(prvx, nxtx, curx);
            T1 ul = std::max(std::min(prvx, nxtx) - d1, curx);
            T1 ll = std::min(std::max(prvx, nxtx) + d2, curx);
            dst0[x] = static_cast<T0>(clamp(avg, ll, ul));
        }
        dst0 += dstride;
        cur0 += cstride;
        prv0 += pstride[0];
        prv1 += pstride[1];
        nxt0 += nstride[0];
        if (STRENGTH > 1) {
            nxt1 += nstride[1];
        }
        if (STRENGTH > 2) {
            prv2 += pstride[2];
            nxt2 += nstride[2];
        }
    }
}


/****************************** SIMD version *********************/

#include "simd.h"
#if defined(__SSE2__)

template <typename T, typename V, int STRENGTH, arch_t ARCH>
static void
proc_simd(uint8_t* dstp, const uint8_t* currp, const uint8_t** prevp,
          const uint8_t** nextp, int dstride, int cstride, int* pstride,
          int* nstride, size_t width, size_t height) noexcept
{
    const uint8_t *prv0, *prv1, *prv2, *nxt0, *nxt1, *nxt2;
    prv0 = prevp[0];
    prv1 = prevp[1];
    nxt0 = nextp[0];
    if (STRENGTH > 1) {
        nxt1 = nextp[1];
    }
    if (STRENGTH > 2) {
        prv2 = prevp[2];
        nxt2 = nextp[2];
    }

    width *= sizeof(T);

    V q = set1<T, V>();

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; x += sizeof(V)) {
            const V curx = load<V>(currp + x);
            V d = abs_diff<T, V>(curx, load<V>(prv1 + x));
            if (STRENGTH > 1) {
                d = min<T, ARCH>(d, abs_diff<T, V>(curx, load<V>(nxt1 + x)));
            }
            if (STRENGTH > 2) {
                d = min<T, ARCH>(d, abs_diff<T, V>(curx, load<V>(prv2 + x)));
                d = min<T, ARCH>(d, abs_diff<T, V>(curx, load<V>(nxt2 + x)));
            }
            const V pr0 = load<V>(prv0 + x);
            const V nx0 = load<V>(nxt0 + x);
            const V ul = max<T, ARCH>(sub<T>(min<T, ARCH>(pr0, nx0), d), curx);
            const V ll = min<T, ARCH>(add<T>(max<T, ARCH>(pr0, nx0), d), curx);
            const V avg = get_avg<T, V>(pr0, nx0, curx, q);
            stream(dstp + x, clamp<T, V, ARCH>(avg, ll, ul));
        }
        prv0 += pstride[0];
        prv1 += pstride[1];
        nxt0 += nstride[0];
        currp += cstride;
        dstp += dstride;
        if (STRENGTH > 1) {
            nxt1 += nstride[1];
        }
        if (STRENGTH > 2) {
            prv2 += pstride[2];
            nxt2 += nstride[2];
        }
    }
}


template <typename T, typename V, arch_t ARCH>
static F_INLINE void
update_diff(const V& x, const V& y, V& d1, V& d2, const V& zero)
{
    const V maxxy = max<T, ARCH>(x, y);
    const V mask = cmpeq<T>(x, maxxy);
    const V d = sub<T>(maxxy, min<T, ARCH>(x, y));
    d1 = blendv<ARCH>(zero, min<T, ARCH>(d, d1), mask);
    d2 = blendv<ARCH>(min<T, ARCH>(d, d2), zero, mask);
}


template <typename T, typename V, int STRENGTH, arch_t ARCH>
static void
proc_a_simd(uint8_t* dstp, const uint8_t* currp, const uint8_t** prevp,
            const uint8_t** nextp, int dstride, int cstride, int* pstride,
            int* nstride, size_t width, size_t height) noexcept
{
    const uint8_t *prv0, *prv1, *prv2, *nxt0, *nxt1, *nxt2;
    prv0 = prevp[0];
    prv1 = prevp[1];
    nxt0 = nextp[0];
    if (STRENGTH > 1) {
        nxt1 = nextp[1];
    }
    if (STRENGTH > 2) {
        prv2 = prevp[2];
        nxt2 = nextp[2];
    }

    width *= sizeof(T);

    V q = set1<T, V>();
    V zero = setzero<V>();

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; x += sizeof(V)) {
            const V curx = load<V>(currp + x);
            V t0 = load<V>(prv1 + x);
            V t1 = max<T, ARCH>(t0, curx);
            V t2 = cmpeq<T>(t0, t1);
            t0 = sub<T>(t1, min<T, ARCH>(t0, curx));
            V d1 = and_reg(t2, t0);
            V d2 = andnot_reg(t2, t0);
            if (STRENGTH > 1) {
                update_diff<T, V, ARCH>(load<V>(nxt1 + x), curx, d1, d2, zero);
            }
            if (STRENGTH > 2) {
                update_diff<T, V, ARCH>(load<V>(prv2 + x), curx, d1, d2, zero);
                update_diff<T, V, ARCH>(load<V>(nxt2 + x), curx, d1, d2, zero);
            }
            const V pr0 = load<V>(prv0 + x);
            const V nx0 = load<V>(nxt0 + x);
            const V ul = max<T, ARCH>(sub<T>(min<T, ARCH>(pr0, nx0), d1), curx);
            const V ll = min<T, ARCH>(add<T>(max<T, ARCH>(pr0, nx0), d2), curx);
            const V avg = get_avg<T, V>(pr0, nx0, curx, q);
            stream(dstp + x, clamp<T, V, ARCH>(avg, ll, ul));
        }
        prv0 += pstride[0];
        prv1 += pstride[1];
        nxt0 += nstride[0];
        currp += cstride;
        dstp += dstride;
        if (STRENGTH > 1) {
            nxt1 += nstride[1];
        }
        if (STRENGTH > 2) {
            prv2 += pstride[2];
            nxt2 += nstride[2];
        }
    }

}

#endif // __SSE2__

#endif

