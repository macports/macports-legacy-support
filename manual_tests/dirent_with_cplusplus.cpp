
/*
 * Copyright (C) 2023 raf <raf@raf.org>
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

/*
 * A recent fix to fdopendir required defining seekdir as a macro to a wrapper
 * function. As it was an argumentless macro, it clashed with the C++ standard
 * library which defines the typename std::ios_base::seekdir. There are other
 * examples of C++ code defining seekdir as an enum type in a similar fashion.
 *
 * The seekdir macro was changed (for C++ only) to a macro with arguments
 * which prevents the clash.
 *
 * This tests the C++ use of the seekdir type in combination with <dirent.h>
 * to make sure that the legacysupport <dirent.h> doesn't break C++ code.
 *
 * This test comes from https://en.cppreference.com/w/cpp/io/ios_base/seekdir
 *
 * All that is required for this test to pass is that it compile.
 *
 * It doesn't matter for the purpose of this project, but the output should
 * look like this:
 *
 *  word1 = Hello,
 *  word2 = Hello,
 *  word3 = World!
 *  word4 = World!
 *  word5 = World!
 *
 *  Note, however, that on my macos-10.6.8 host, the output is:
 *
 *  word1 = Hello,
 *  word2 = Hello,
 *  word3 = World!
 *  word4 = 
 *  word5 = 
 *
 * But that's the output with or without <dirent.h> so the cause lies
 * elsewhere.
 */

#include <iostream>
#include <string>
#include <sstream>

#include <dirent.h>
 
int main()
{
    std::istringstream in("Hello, World!");
    std::string word1, word2, word3, word4, word5;
    std::ios_base::seekdir begdir = std::ios_base::beg;
    std::ios_base::seekdir curdir = std::ios_base::cur;
    std::ios_base::seekdir enddir = std::ios_base::end;
 
    in >> word1;
    in.seekg(0, begdir); // <- rewind
    in >> word2;
    in.seekg(1, curdir); // -> seek from cur pos toward the end
    in >> word3;
    in.seekg(-6, curdir); // <- seek from cur pos (end) toward begin
    in >> word4;
    in.seekg(-6, enddir); // <- seek from end toward begin
    in >> word5;
 
    std::cout << "word1 = " << word1 << '\n'
              << "word2 = " << word2 << '\n'
              << "word3 = " << word3 << '\n'
              << "word4 = " << word4 << '\n'
              << "word5 = " << word5 << '\n';

    return 0;
}

