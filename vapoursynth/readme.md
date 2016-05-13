## ReduceFlicker for VapourSynth
	This is a port of Avisynth's ReduceFlicker which written by Rainer Wittmann.
	This plugin has only ReduceFlicker(). ReduceFluctuation() and LockClense() are not implemented.

### Requirements:
	- VapourSynth r30 or later

### Syntax:
	rdfl.ReduceFlicker(clip clip[, int strength, int aggressive, int[] planes, int opt])

#### clip:
	All formats except half precision are supported.

#### strength:
	Specify the strength of ReduceFlicker. Higher values mean more aggressive operation.

	1 - makes use of 4(current + 2*previous + 1*next) frames .
	2(default) - makes use of 5(current + 2*previous + 2*next) frames.
	3 - makes use of 7(current + 3*previous + 3*next) frames.

#### aggressive:
	If set this to 1, then a significantly more aggressive variant of the algorithm is selected.
	Default value is 0.

#### planes:
	Whether planes will be processed or not. If set this to 0, the plane will be copied from source clip.
	Default values are [1, 1, 1](all planes will be processed).
	
	examples
		planes=[0, 1, 1] -> chroma (or G/B planes) will be processed.
		planes=[0, 0, 0] -> do nothing (all planes will be copied from source).

#### opt:
	Controls which cpu optimizations are used.
	Currently, this filter has four routines.

	0 - Use C++ routine.
	1 - Use SSE2/SSE routine. If cpu does not have SSE2, fallback to 0.
	2 - Use SSE4.1/SSE2/SSE routine. If cpu does not have SSE4.1, fallback to 1.
	3(default) - Use AVX2/AVX routine. If cpu does not have AVX2, fallback to 2.

### Lisence:
	LGPLv2.1 or later.

### Source code:
	https://github.com/chikuzen/ReduceFlicker/vapoursynth/
