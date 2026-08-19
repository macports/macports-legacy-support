/*
 * Copyright (c) 2026 Frederick H. G. Wright II <fw@fwright.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* MP support header */
#include "MacportsLegacySupport.h"

#if __MPLS_LIB_SUPPORT_EXP10__

/*
 * These implementations are slightly tweaked versions from the musl project.
 * See: https://git.musl-libc.org/cgit/musl/tree/src/math
 */

/*
----------------------------------------------------------------------
Copyright © 2005-2020 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
----------------------------------------------------------------------
*/

#include <math.h>
#include <stdint.h>

double __exp10(double x)
{
	static const double p10[] = {
		1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10,
		1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1,
		1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
		1e10, 1e11, 1e12, 1e13, 1e14, 1e15
	};
	double n, y = modf(x, &n);
	union {double f; uint64_t i;} u = {n};
	/* fabs(n) < 16 without raising invalid on nan */
	if ((u.i>>52 & 0x7ff) < 0x3ff+4) {
		if (!y) return p10[(int)n+15];
		y = exp2(3.32192809488736234787031942948939 * y);
		return y * p10[(int)n+15];
	}
	return pow(10.0, x);
}

float __exp10f(float x)
{
	static const float p10[] = {
		1e-7f, 1e-6f, 1e-5f, 1e-4f, 1e-3f, 1e-2f, 1e-1f,
		1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7
	};
	float n, y = modff(x, &n);
	union {float f; uint32_t i;} u = {n};
	/* fabsf(n) < 8 without raising invalid on nan */
	if ((u.i>>23 & 0xff) < 0x7f+3) {
		if (!y) return p10[(int)n+7];
		y = exp2f(3.32192809488736234787031942948939f * y);
		return y * p10[(int)n+7];
	}
	return exp2(3.32192809488736234787031942948939 * x);
}

#endif  /* __MPLS_LIB_SUPPORT_EXP10__ */
