/*
simd.h

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


#ifndef REDUCE_FLICKER_SIMD_H
#define REDUCE_FLICKER_SIMD_H

#include <cstdint>
#include <immintrin.h>

#define SFINLINE static __forceinline



template <typename T> T set1(int8_t x);

template <>
SFINLINE __m128i set1<__m128i>(int8_t x)
{
    return _mm_set1_epi8(x);
}

template <>
SFINLINE __m256i set1<__m256i>(int8_t x)
{
    return _mm256_set1_epi8(x);
}

template <typename T> T load(const uint8_t* p);

template <>
SFINLINE __m128i load<__m128i>(const uint8_t* p)
{
    return _mm_load_si128(reinterpret_cast<const __m128i*>(p));
}

template <>
SFINLINE __m256i load<__m256i>(const uint8_t* p)
{
    return _mm256_load_si256(reinterpret_cast<const __m256i*>(p));
}

SFINLINE void stream(uint8_t* p, const __m128i& x)
{
    _mm_stream_si128(reinterpret_cast<__m128i*>(p), x);
}

SFINLINE void stream(uint8_t* p, const __m256i& x)
{
    _mm256_stream_si256(reinterpret_cast<__m256i*>(p), x);
}

SFINLINE __m128i adds(const __m128i& x, const __m128i& y)
{
    return _mm_adds_epu8(x, y);
}

SFINLINE __m256i adds(const __m256i& x, const __m256i& y)
{
    return _mm256_adds_epu8(x, y);
}

SFINLINE __m128i subs(const __m128i& x, const __m128i& y)
{
    return _mm_subs_epu8(x, y);
}

SFINLINE __m256i subs(const __m256i& x, const __m256i& y)
{
    return _mm256_subs_epu8(x, y);
}

SFINLINE __m128i or_reg(const __m128i& x, const __m128i& y)
{
    return _mm_or_si128(x, y);
}

SFINLINE __m256i or_reg(const __m256i& x, const __m256i& y)
{
    return _mm256_or_si256(x, y);
}

SFINLINE __m128i and_reg(const __m128i& x, const __m128i& y)
{
    return _mm_and_si128(x, y);
}

SFINLINE __m256i and_reg(const __m256i& x, const __m256i& y)
{
    return _mm256_and_si256(x, y);
}

SFINLINE __m128i andnot_reg(const __m128i& x, const __m128i& y)
{
    return _mm_andnot_si128(x, y);
}

SFINLINE __m256i andnot_reg(const __m256i& x, const __m256i& y)
{
    return _mm256_andnot_si256(x, y);
}

SFINLINE __m128i xor_reg(const __m128i& x, const __m128i& y)
{
    return _mm_xor_si128(x, y);
}

SFINLINE __m256i xor_reg(const __m256i& x, const __m256i& y)
{
    return _mm256_xor_si256(x, y);
}

SFINLINE __m128i cmpeq(const __m128i& x, const __m128i& y)
{
    return _mm_cmpeq_epi8(x, y);
}

SFINLINE __m256i cmpeq(const __m256i& x, const __m256i& y)
{
    return _mm256_cmpeq_epi8(x, y);
}

SFINLINE __m128i min(const __m128i& x, const __m128i& y)
{
    return _mm_min_epu8(x, y);
}

SFINLINE __m256i min(const __m256i& x, const __m256i& y)
{
    return _mm256_min_epu8(x, y);
}

SFINLINE __m128i max(const __m128i& x, const __m128i& y)
{
    return _mm_max_epu8(x, y);
}

SFINLINE __m256i max(const __m256i& x, const __m256i& y)
{
    return _mm256_max_epu8(x, y);
}

SFINLINE __m128i average(const __m128i& x, const __m128i& y)
{
    return _mm_avg_epu8(x, y);
}

SFINLINE __m256i average(const __m256i& x, const __m256i& y)
{
    return _mm256_avg_epu8(x, y);
}

SFINLINE __m128i blendv(const __m128i& x, const __m128i& y, const __m128i& m)
{
    return or_reg(and_reg(m, y), andnot_reg(m, x));
}

SFINLINE __m256i blendv(const __m256i& x, const __m256i& y, const __m256i& m)
{
    return _mm256_blendv_epi8(x, y, m);
}

template <typename T>
SFINLINE T absdiff(const T& x, const T& y)
{
    return or_reg(subs(x, y), subs(y, x));
}

template <typename T>
SFINLINE T clamp(const T& val, const T& minimum, const T& maximum)
{
    return min(max(val, minimum), maximum);
}


#endif

