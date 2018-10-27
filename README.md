
# MacPorts Support for legacy OSX versions

Installs a number of wrapper headers around system headers that add
functionality missing in various older OSX releases.

Installed headers use the `include_next` pre-processor feature to add
the missing features and then forward include the original header.
So using these headers instead of the originals should be transparent.

Missing functions are compiled into a library that must also be linked
into any builds using the wrapped headers. This is handled in macPorts
via the legacysupport PortGroup.

Wrapped headers area

 - `cmath`       : Adds declaration of various `long long` methods missing in OSX10.6 and older.
 - `stdio.h`     : Adds `getline` and `getdelim` functions missing in OSX10.6 and older.
 - `string.h`    : Adds `strnlen`, `strndup` and `memmem` functions missing in OSX10.6 and older.
 - `time.h`      : Adds `clock_gettime` function missing in OSX10.11 and older.
 - `wchar.h`     : Adds `wcsdup` function missing in OSX10.6 and older.
 - `sys/fcntl.h` : Adds missing `O_CLOEXEC` definition as required.
 - `sys/mman.h`  : Adds missing `MAP_ANONYMOUS` definition as required.
