All functions provided by legacy-support are C functions.  Use of them from C++
is via the C linkage.  Thus, not only is the entire library written in plain C,
but there is also no need for any tests to be written in C++.

In addition, Apple made the C++ compiler requirements stricter in the macOS 15
SDK, which would have been the limiting factor in compatibility with any
automatic tests written in C++.  Consequently, the three former C++ tests have
been replaced by C versions, with the old C++ versions being moved to the manual
test category.

The three affected tests are:

os_unfair_lock:

This had a comment indicating that it needed to be in C++, with a commit
reference supposedly explaining that, but neither that commit nor the ticket
that it references contains any such explanation.

dirent_with_cplusplus:

The former fdopendir() implementation relied on a macro that collided with C++
headers, causing errors.  The test was created to demonstrate that problem and
its fix.  It was written in C++ since that was how the problem was actually
discovered, though a test using plain C probably could have been constructed. 
Meanwhile, the fdopendir() implementation has been rewritten in such a way that
the aforementioned problem no longer exists.  For that reason, the test was
replaced with a simple include-only test, rather than rewriting the former test
in C.

time_cpp:

This is a test of clock_gettime() and clock_getres(), which was written in C++
for no good reason at all.  It has been rewritten in C, with some additional
improvements, including the ability to detect a previously undiscovered bug.
