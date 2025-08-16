
/*
 * Copyright (c) 2018 Christian Cornelssen
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

#include <wchar.h>
#include <assert.h>

int main() {
    assert (wcscasecmp(L"", L"") == 0);
    assert (wcscasecmp(L"a", L"") > 0);
    assert (wcscasecmp(L"", L"a") < 0);
    assert (wcscasecmp(L"a", L"b") < 0);
    assert (wcscasecmp(L"b", L"a") > 0);
    assert (wcscasecmp(L"ab", L"b") < 0);
    assert (wcscasecmp(L"b", L"ab") > 0);
    assert (wcscasecmp(L"ab", L"ba") < 0);
    assert (wcscasecmp(L"ba", L"ab") > 0);
    assert (wcscasecmp(L"a", L"a") == 0);
    assert (wcscasecmp(L"A", L"a") == 0);
    assert (wcscasecmp(L"a", L"A") == 0);
    assert (wcscasecmp(L"A", L"A") == 0);
    assert (wcscasecmp(L"A", L"ab") < 0);
    assert (wcscasecmp(L"a", L"Ab") < 0);
    assert (wcscasecmp(L"Ab", L"a") > 0);
    assert (wcscasecmp(L"ab", L"A") > 0);
    assert (wcscasecmp(L"a", L"B") < 0);
    assert (wcscasecmp(L"b", L"A") > 0);
    assert (wcscasecmp(L"Ab", L"b") < 0);
    assert (wcscasecmp(L"B", L"ab") > 0);
    assert (wcscasecmp(L"ab", L"Ba") < 0);
    assert (wcscasecmp(L"ba", L"Ab") > 0);
    assert (wcscasecmp(L"ab", L"ab") == 0);
    assert (wcscasecmp(L"aB", L"ab") == 0);
    assert (wcscasecmp(L"Ab", L"ab") == 0);
    assert (wcscasecmp(L"AB", L"ab") == 0);
    assert (wcscasecmp(L"ab", L"Ab") == 0);
    assert (wcscasecmp(L"aB", L"Ab") == 0);
    assert (wcscasecmp(L"Ab", L"Ab") == 0);
    assert (wcscasecmp(L"AB", L"Ab") == 0);
    assert (wcscasecmp(L"ab", L"aB") == 0);
    assert (wcscasecmp(L"aB", L"aB") == 0);
    assert (wcscasecmp(L"Ab", L"aB") == 0);
    assert (wcscasecmp(L"AB", L"aB") == 0);
    assert (wcscasecmp(L"ab", L"AB") == 0);
    assert (wcscasecmp(L"aB", L"AB") == 0);
    assert (wcscasecmp(L"Ab", L"AB") == 0);
    assert (wcscasecmp(L"AB", L"AB") == 0);

    assert (wcsncasecmp(L"", L"", 0) == 0);
    assert (wcsncasecmp(L"a", L"", 1) > 0);
    assert (wcsncasecmp(L"", L"a", 1) < 0);
    assert (wcsncasecmp(L"a", L"b", 1) < 0);
    assert (wcsncasecmp(L"b", L"a", 1) > 0);
    assert (wcsncasecmp(L"ab", L"b", 2) < 0);
    assert (wcsncasecmp(L"b", L"ab", 2) > 0);
    assert (wcsncasecmp(L"ab", L"ba", 2) < 0);
    assert (wcsncasecmp(L"ba", L"ab", 2) > 0);
    assert (wcsncasecmp(L"a", L"a", 1) == 0);
    assert (wcsncasecmp(L"A", L"a", 1) == 0);
    assert (wcsncasecmp(L"a", L"A", 1) == 0);
    assert (wcsncasecmp(L"A", L"A", 1) == 0);
    assert (wcsncasecmp(L"A", L"ab", 2) < 0);
    assert (wcsncasecmp(L"a", L"Ab", 2) < 0);
    assert (wcsncasecmp(L"Ab", L"a", 2) > 0);
    assert (wcsncasecmp(L"ab", L"A", 2) > 0);
    assert (wcsncasecmp(L"a", L"B", 1) < 0);
    assert (wcsncasecmp(L"b", L"A", 1) > 0);
    assert (wcsncasecmp(L"Ab", L"b", 2) < 0);
    assert (wcsncasecmp(L"B", L"ab", 2) > 0);
    assert (wcsncasecmp(L"ab", L"Ba", 2) < 0);
    assert (wcsncasecmp(L"ba", L"Ab", 2) > 0);
    assert (wcsncasecmp(L"ab", L"ab", 2) == 0);
    assert (wcsncasecmp(L"aB", L"ab", 2) == 0);
    assert (wcsncasecmp(L"Ab", L"ab", 2) == 0);
    assert (wcsncasecmp(L"AB", L"ab", 2) == 0);
    assert (wcsncasecmp(L"ab", L"Ab", 2) == 0);
    assert (wcsncasecmp(L"aB", L"Ab", 2) == 0);
    assert (wcsncasecmp(L"Ab", L"Ab", 2) == 0);
    assert (wcsncasecmp(L"AB", L"Ab", 2) == 0);
    assert (wcsncasecmp(L"ab", L"aB", 2) == 0);
    assert (wcsncasecmp(L"aB", L"aB", 2) == 0);
    assert (wcsncasecmp(L"Ab", L"aB", 2) == 0);
    assert (wcsncasecmp(L"AB", L"aB", 2) == 0);
    assert (wcsncasecmp(L"ab", L"AB", 2) == 0);
    assert (wcsncasecmp(L"aB", L"AB", 2) == 0);
    assert (wcsncasecmp(L"Ab", L"AB", 2) == 0);
    assert (wcsncasecmp(L"AB", L"AB", 2) == 0);

    assert (wcsncasecmp(L"x", L"y", 0) == 0);
    assert (wcsncasecmp(L"ax", L"y", 1) < 0);
    assert (wcsncasecmp(L"x", L"ay", 1) > 0);
    assert (wcsncasecmp(L"ax", L"by", 1) < 0);
    assert (wcsncasecmp(L"bx", L"ay", 1) > 0);
    assert (wcsncasecmp(L"abx", L"by", 2) < 0);
    assert (wcsncasecmp(L"bx", L"aby", 2) > 0);
    assert (wcsncasecmp(L"abx", L"bay", 2) < 0);
    assert (wcsncasecmp(L"bax", L"aby", 2) > 0);
    assert (wcsncasecmp(L"ax", L"ay", 1) == 0);
    assert (wcsncasecmp(L"Ax", L"ay", 1) == 0);
    assert (wcsncasecmp(L"ax", L"Ay", 1) == 0);
    assert (wcsncasecmp(L"Ax", L"Ay", 1) == 0);
    assert (wcsncasecmp(L"Ax", L"aby", 2) > 0);
    assert (wcsncasecmp(L"ax", L"Aby", 2) > 0);
    assert (wcsncasecmp(L"Abx", L"ay", 2) < 0);
    assert (wcsncasecmp(L"abx", L"Ay", 2) < 0);
    assert (wcsncasecmp(L"ax", L"By", 1) < 0);
    assert (wcsncasecmp(L"bx", L"Ay", 1) > 0);
    assert (wcsncasecmp(L"Abx", L"by", 2) < 0);
    assert (wcsncasecmp(L"Bx", L"aby", 2) > 0);
    assert (wcsncasecmp(L"abx", L"Bay", 2) < 0);
    assert (wcsncasecmp(L"bax", L"Aby", 2) > 0);
    assert (wcsncasecmp(L"abx", L"aby", 2) == 0);
    assert (wcsncasecmp(L"aBx", L"aby", 2) == 0);
    assert (wcsncasecmp(L"Abx", L"aby", 2) == 0);
    assert (wcsncasecmp(L"ABx", L"aby", 2) == 0);
    assert (wcsncasecmp(L"abx", L"Aby", 2) == 0);
    assert (wcsncasecmp(L"aBx", L"Aby", 2) == 0);
    assert (wcsncasecmp(L"Abx", L"Aby", 2) == 0);
    assert (wcsncasecmp(L"ABx", L"Aby", 2) == 0);
    assert (wcsncasecmp(L"abx", L"aBy", 2) == 0);
    assert (wcsncasecmp(L"aBx", L"aBy", 2) == 0);
    assert (wcsncasecmp(L"Abx", L"aBy", 2) == 0);
    assert (wcsncasecmp(L"ABx", L"aBy", 2) == 0);
    assert (wcsncasecmp(L"abx", L"ABy", 2) == 0);
    assert (wcsncasecmp(L"aBx", L"ABy", 2) == 0);
    assert (wcsncasecmp(L"Abx", L"ABy", 2) == 0);
    assert (wcsncasecmp(L"ABx", L"ABy", 2) == 0);

    assert (wcsncasecmp(L"y", L"x", 0) == 0);
    assert (wcsncasecmp(L"ay", L"x", 1) < 0);
    assert (wcsncasecmp(L"y", L"ax", 1) > 0);
    assert (wcsncasecmp(L"ay", L"bx", 1) < 0);
    assert (wcsncasecmp(L"by", L"ax", 1) > 0);
    assert (wcsncasecmp(L"aby", L"bx", 2) < 0);
    assert (wcsncasecmp(L"by", L"abx", 2) > 0);
    assert (wcsncasecmp(L"aby", L"bax", 2) < 0);
    assert (wcsncasecmp(L"bay", L"abx", 2) > 0);
    assert (wcsncasecmp(L"ay", L"ax", 1) == 0);
    assert (wcsncasecmp(L"Ay", L"ax", 1) == 0);
    assert (wcsncasecmp(L"ay", L"Ax", 1) == 0);
    assert (wcsncasecmp(L"Ay", L"Ax", 1) == 0);
    assert (wcsncasecmp(L"Ay", L"abx", 2) > 0);
    assert (wcsncasecmp(L"ay", L"Abx", 2) > 0);
    assert (wcsncasecmp(L"Aby", L"ax", 2) < 0);
    assert (wcsncasecmp(L"aby", L"Ax", 2) < 0);
    assert (wcsncasecmp(L"ay", L"Bx", 1) < 0);
    assert (wcsncasecmp(L"by", L"Ax", 1) > 0);
    assert (wcsncasecmp(L"Aby", L"bx", 2) < 0);
    assert (wcsncasecmp(L"By", L"abx", 2) > 0);
    assert (wcsncasecmp(L"aby", L"Bax", 2) < 0);
    assert (wcsncasecmp(L"bay", L"Abx", 2) > 0);
    assert (wcsncasecmp(L"aby", L"abx", 2) == 0);
    assert (wcsncasecmp(L"aBy", L"abx", 2) == 0);
    assert (wcsncasecmp(L"Aby", L"abx", 2) == 0);
    assert (wcsncasecmp(L"ABy", L"abx", 2) == 0);
    assert (wcsncasecmp(L"aby", L"Abx", 2) == 0);
    assert (wcsncasecmp(L"aBy", L"Abx", 2) == 0);
    assert (wcsncasecmp(L"Aby", L"Abx", 2) == 0);
    assert (wcsncasecmp(L"ABy", L"Abx", 2) == 0);
    assert (wcsncasecmp(L"aby", L"aBx", 2) == 0);
    assert (wcsncasecmp(L"aBy", L"aBx", 2) == 0);
    assert (wcsncasecmp(L"Aby", L"aBx", 2) == 0);
    assert (wcsncasecmp(L"ABy", L"aBx", 2) == 0);
    assert (wcsncasecmp(L"aby", L"ABx", 2) == 0);
    assert (wcsncasecmp(L"aBy", L"ABx", 2) == 0);
    assert (wcsncasecmp(L"Aby", L"ABx", 2) == 0);
    assert (wcsncasecmp(L"ABy", L"ABx", 2) == 0);
    return 0;
}
