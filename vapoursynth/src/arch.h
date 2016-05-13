/*
arch.h: Copyright (C) 2016  Oka Motofumi

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


#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

#if defined(_M_IX86) || defined(_M_AMD64) || defined(__i686) || defined(__x86_64)
    #define INTEL_X86_CPU
#if !defined(__GNUC__)
    #define __AVX2__
    #define __SSE4_1__
    #define __SSE2__
#endif // __GNUC__

#endif // X86


#if defined(__AVX2__)
    #include <immintrin.h>
#elif defined(__SSE4_1__)
    #include <smmintrin.h>
#elif defined(__SSSE3__)
    #include <pmmintrin.h>
#elif defined(__SSE2__)
    #include <emmintrin.h>
#endif


#if defined(__GNUC__)
    #define SFINLINE inline __attribute__((always_inline))
#else
    #define SFINLINE static __forceinline
#endif


enum arch_t {
    NO_SIMD,
    USE_SSE2,
    USE_SSSE3,
    USE_SSE41,
    USE_AVX2,
};


#if defined(INTEL_X86_CPU)
    extern bool has_sse2(void);
    extern bool has_ssse3(void);
    extern bool has_sse41(void);
    extern bool has_avx2(void);
#endif


static inline arch_t get_arch(int opt)
{
#if !defined(__SSE2__)
    return NO_SIMD
#else
    if (opt == 0 || !has_sse2()) {
        return NO_SIMD;
    }
#if !defined(__SSE4_1__)
    return USE_SSE2;
#else
    if (opt == 1 || !has_sse41()) {
        return USE_SSE2;
    }
#if !defined(__AVX2__)
    return USE_SSE41;
#else
    if (opt == 2 || !has_avx2()) {
        return USE_SSE41;
    }
    return USE_AVX2;
#endif // __AVX2__
#endif // __SSE4_1__
#endif // __SSE2__
}

#endif //ARCHITECTURE_H
