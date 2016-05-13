/*
proc_filter.cpp

This file is a part of ReduceFlicker.

Copyright (C) 2016 OKA Motofumi

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
*/


#include <algorithm>
#include "ReduceFlicker.h"


static inline int abs_diff(int x, int y)
{
    return x < y ? y - x : x - y;
}

static inline int average(int x, int y)
{
    return (x + y + 1) / 2;
}

static inline int clamp(int x, int minimum, int maximum)
{
    return std::min(std::max(x, minimum), maximum);
}


template <int STRENGTH>
static void __stdcall
proc_c(uint8_t* dstp, const uint8_t** prevp, const uint8_t* currp,
       const uint8_t** nextp, const int dpitch, const int* ppitch,
       const int cpitch, const int* npitch, const int width, const int height)
       noexcept
{
    using std::min;
    using std::max;

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

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int currx = static_cast<int>(currp[x]);
            int d  = abs_diff(currx, prv1[x]);
            if (STRENGTH > 1) {
                d = min(d, abs_diff(currx, nxt1[x]));
            }
            if (STRENGTH > 2) {
                d = min(d, abs_diff(currx, prv2[x]));
                d = min(d, abs_diff(currx, nxt2[x]));
            }
            int avg = max(average(prv0[x], nxt0[x]) - 1, 0);
            avg = average(avg, currx);
            int ul = max(min(prv0[x], nxt0[x]) - d, currx);
            int ll = min(max(prv0[x], nxt0[x]) + d, currx);
            dstp[x] = clamp(avg, ll, ul);
        }
        prv0 += ppitch[0];
        prv1 += ppitch[1];
        nxt0 += npitch[0];
        currp += cpitch;
        dstp += dpitch;
        if (STRENGTH > 1) {
            nxt1 += npitch[1];
        }
        if (STRENGTH > 2) {
            prv2 += ppitch[2];
            nxt2 += npitch[2];
        }
    }
}


static inline void update_diff(int x, int y, int& d1, int& d2)
{
    int d = x - y;
    if (d >= 0) {
        d2 = 0;
        d1 = std::min(d, d1);
    } else {
        d1 = 0;
        d2 = std::min(-d, d2);
    }
}


template <int STRENGTH>
static void __stdcall
proc_a_c(uint8_t* dstp, const uint8_t** prevp, const uint8_t* currp,
         const uint8_t** nextp, const int dpitch, const int* ppitch,
         const int cpitch, const int* npitch, const int width, const int height)
         noexcept
{
    using std::min;
    using std::max;

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

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int currx = static_cast<int>(currp[x]);
            int d1 = prv1[x] - currx;
            int d2 = 0;
            if (d1 < 0) {
                d2 = -d1;
                d1 = 0;
            }
            if (STRENGTH > 1) {
                update_diff(nxt1[x], currx, d1, d2);
            }
            if (STRENGTH > 2) {
                update_diff(prv2[x], currx, d1, d2);
                update_diff(nxt2[x], currx, d1, d2);
            }

            int avg = max(average(prv0[x], nxt0[x]) - 1, 0);
            avg = average(avg, currx);
            int ul = max(min(prv0[x], nxt0[x]) - d1, currx);
            int ll = min(max(prv0[x], nxt0[x]) + d2, currx);
            dstp[x] = clamp(avg, ll, ul);
        }
        prv0 += ppitch[0];
        prv1 += ppitch[1];
        nxt0 += npitch[0];
        currp += cpitch;
        dstp += dpitch;
        if (STRENGTH > 1) {
            nxt1 += npitch[1];
        }
        if (STRENGTH > 2) {
            prv2 += ppitch[2];
            nxt2 += npitch[2];
        }
    }
}


/* SIMD VERSION ------------------------------------------------------------*/

#include "simd.h"


template <typename T, int STRENGTH>
static void __stdcall
proc_simd(uint8_t* dstp, const uint8_t** prevp, const uint8_t* currp,
          const uint8_t** nextp, const int dpitch, const int* ppitch,
          const int cpitch, const int* npitch, const int width,
          const int height) noexcept
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

    const T one = set1<T>(1);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; x += sizeof(T)) {
            const T currx = load<T>(currp + x);
            T d = absdiff(currx, load<T>(prv1 + x));
            if (STRENGTH > 1) {
                d = min(d, absdiff<T>(currx, load<T>(nxt1 + x)));
            }
            if (STRENGTH > 2) {
                d = min(d, absdiff(currx, load<T>(prv2 + x)));
                d = min(d, absdiff(currx, load<T>(nxt2 + x)));
            }
            const T pr0 = load<T>(prv0 + x);
            const T nx0 = load<T>(nxt0 + x);
            const T ul = max(subs(min(pr0, nx0), d), currx);
            const T ll = min(adds(max(pr0, nx0), d), currx);
            const T avg = average(subs(average(pr0, nx0), one), currx);
            stream(dstp + x, clamp(avg, ll, ul));
        }
        prv0 += ppitch[0];
        prv1 += ppitch[1];
        nxt0 += npitch[0];
        currp += cpitch;
        dstp += dpitch;
        if (STRENGTH > 1) {
            nxt1 += npitch[1];
        }
        if (STRENGTH > 2) {
            prv2 += ppitch[2];
            nxt2 += npitch[2];
        }
    }
}


template <typename T>
SFINLINE void update_diff(const T& x, const T& y, T& d1, T& d2, const T& zero)
{
    const T maxxy = max(x, y);
    const T mask = cmpeq(x, maxxy);
    const T d = subs(maxxy, min(x, y));
    d1 = blendv(zero, min(d, d1), mask);
    d2 = blendv(min(d, d2), zero, mask);
}


template <typename T, int STRENGTH>
static void __stdcall
proc_a_simd(uint8_t* dstp, const uint8_t** prevp, const uint8_t* currp,
            const uint8_t** nextp, const int dpitch, const int* ppitch,
            const int cpitch, const int* npitch, const int width,
            const int height) noexcept
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

    const T one = set1<T>(1);
    const T zero = xor_reg(one, one);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; x += sizeof(T)) {
            const T currx = load<T>(currp + x);
            T t0 = load<T>(prv1 + x);
            T t1 = max(t0, currx);
            T t2 = cmpeq(t0, t1);
            t0 = subs(t1, min(t0, currx));
            T d1 = and_reg(t2, t0);
            T d2 = andnot_reg(t2, t0);
            if (STRENGTH > 1) {
                update_diff(load<T>(nxt1 + x), currx, d1, d2, zero);
            }
            if (STRENGTH > 2) {
                update_diff(load<T>(prv2 + x), currx, d1, d2, zero);
                update_diff(load<T>(nxt2 + x), currx, d1, d2, zero);
            }

            const T pr0 = load<T>(prv0 + x);
            const T nx0 = load<T>(nxt0 + x);
            const T ul = max(subs(min(pr0, nx0), d1), currx);
            const T ll = min(adds(max(pr0, nx0), d2), currx);
            const T avg = average(subs(average(pr0, nx0), one), currx);
            stream(dstp + x, clamp(avg, ll, ul));
        }
        prv0 += ppitch[0];
        prv1 += ppitch[1];
        nxt0 += npitch[0];
        currp += cpitch;
        dstp += dpitch;
        if (STRENGTH > 1) {
            nxt1 += npitch[1];
        }
        if (STRENGTH > 2) {
            prv2 += ppitch[2];
            nxt2 += npitch[2];
        }
    }
}


/* function selector -------------------------------------------------------*/

#include <map>
#include <tuple>


proc_filter_t
get_main_proc(int strength, bool aggressive, arch_t arch)
{
    using std::make_tuple;

    std::map<std::tuple<int, bool, arch_t>, proc_filter_t> func;

    func[make_tuple(1, false, NO_SIMD)] = proc_c<1>;
    func[make_tuple(2, false, NO_SIMD)] = proc_c<2>;
    func[make_tuple(3, false, NO_SIMD)] = proc_c<3>;

    func[make_tuple(1, true, NO_SIMD)] = proc_a_c<1>;
    func[make_tuple(2, true, NO_SIMD)] = proc_a_c<2>;
    func[make_tuple(3, true, NO_SIMD)] = proc_a_c<3>;

    func[make_tuple(1, false, USE_SSE2)] = proc_simd<__m128i, 1>;
    func[make_tuple(2, false, USE_SSE2)] = proc_simd<__m128i, 2>;
    func[make_tuple(3, false, USE_SSE2)] = proc_simd<__m128i, 3>;

    func[make_tuple(1, true, USE_SSE2)] = proc_a_simd<__m128i, 1>;
    func[make_tuple(2, true, USE_SSE2)] = proc_a_simd<__m128i, 2>;
    func[make_tuple(3, true, USE_SSE2)] = proc_a_simd<__m128i, 3>;

    func[make_tuple(1, false, USE_AVX2)] = proc_simd<__m256i, 1>;
    func[make_tuple(2, false, USE_AVX2)] = proc_simd<__m256i, 2>;
    func[make_tuple(3, false, USE_AVX2)] = proc_simd<__m256i, 3>;

    func[make_tuple(1, true, USE_AVX2)] = proc_a_simd<__m256i, 1>;
    func[make_tuple(2, true, USE_AVX2)] = proc_a_simd<__m256i, 2>;
    func[make_tuple(3, true, USE_AVX2)] = proc_a_simd<__m256i, 3>;

    return func[make_tuple(strength, aggressive, arch)];
}


