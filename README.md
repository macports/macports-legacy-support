# MacPorts Support for Legacy OSX Versions

## Features

Installs a number of wrapper headers around system headers that add
functionality missing in various older OSX releases.

Installed headers use the `include_next` pre-processor feature to add
the missing features and then forward include the original header.
So using these headers instead of the originals should be transparent.

Missing functions are compiled into a library that must also be linked
into any builds using the wrapped headers. This is handled in
[MacPorts](https://github.com/macports) via the `legacysupport` PortGroup.

Wrapped headers are:

<table>
  <tr>
    <th>Header File</th>
    <th>Feature</th>
    <th>Max Version Needing Feature</th>
  </tr>
  <tr>
    <td><code>cmath</code></td>
    <td>Adds the same functions as those provided by the herein <code>math.h</code>,
        in namespace <code>std::</code>.</td>
    <td>see <code>math.h</code></td>
  </tr>
  <tr>
    <td><code>dirent.h</code></td>
    <td>Adds <code>fdopendir</code> function</td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td><code>math.h</code></td>
    <td>Adds declaration of various <code>long long</code> methods (OSX10.6) and <code>__sincos</code> (macOS10.8)</td>
    <td>OSX10.6(8), GCC 8</td>
  </tr>
  <tr>
    <td><code>netdb.h</code></td>
    <td>Adds declaration of <code>AI_NUMERICSERV</code></td>
    <td>OSX10.5</td>
  </tr>
 <tr>
    <td><code>pthread.h</code></td>
    <td>Adds <code>PTHREAD_RWLOCK_INITIALIZER</code></td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td><code>stdio.h</code></td>
    <td>Adds <code>getline</code> and <code>getdelim</code> functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td rowspan="2"><code>stdlib.h</code></td>
    <td>Adds <code>posix_memalign</code> functional replacement, and wraps <code>realpath</code>
        to accept a <code>NULL</code> buffer argument</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td>Adds <code>arc4random_uniform</code> and <code>arc4random_buf</code> functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td><code>string.h</code></td>
    <td>Adds <code>strnlen</code>, <code>strndup</code> and <code>memmem</code> functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td><code>strings.h</code></td>
    <td>Adds <code>fls,flsl,ffsl</code>(OSX10.4) and <code>flsll,ffsll</code>(macOS10.8) functions</td>
    <td>OSX10.4(8)</td>
  </tr>
  <tr>
    <td><code>time.h</code></td>
    <td>Adds <code>clock_gettime</code> function</td>
    <td>OSX10.11</td>
  </tr>
  <tr>
    <td><code>wchar.h</code></td>
    <td>Adds <code>wcsdup</code>, <code>wcsnlen</code>, <code>wcpcpy</code>,
        <code>wcpncpy</code>, <code>wcscasecmp</code>, and <code>wcsncasecmp</code>
        functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td><code>xlocale/_wchar.h</code></td>
    <td>Adds <code>wcscasecmp_l</code>, <code>wcsncasecmp_l</code> functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td rowspan="2"><code>sys/fcntl.h</code></td>
    <td>Adds missing <code>O_CLOEXEC</code>, <code>AT_FDCWD</code>, <code>AT_EACCESS</code>,
        <code>AT_SYMLINK_NOFOLLOW</code>, <code>AT_SYMLINK_FOLLOW</code>, and
        <code>AT_REMOVEDIR</code> definitions</td>
    <td>as required (?)</td>
  </tr>
  <tr>
    <td>Adds <code>openat</code> function</td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td><code>sys/fsgetpath.h</code></td>
    <td>Adds missing <code>fsgetpath</code> function</td>
    <td>OSX10.12</td>
  </tr>
  <tr>
    <td><code>sys/mman.h</code></td>
    <td>Adds missing <code>MAP_ANONYMOUS</code> definition</td>
    <td>OSX10.10</td>
  </tr>
  <tr>
    <td><code>sys/stdio.h</code></td>
    <td>Adds <code>renameat</code> function</td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td rowspan="2"><code>sys/stat.h</code></td>
    <td>Adds <code>fchmodat</code>, <code>fstatat</code>, <code>fstatat64</code> (if required, and on 10.5+),
        and <code>mkdirat</code> functions</td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td>Adds <code>lchmod</code> function</td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td rowspan="3"><code>sys/unistd.h</code></td>
    <td>Adds <code>getattrlistat</code>, <code>readlinkat</code>, <code>faccessat</code>,
        <code>fchownat</code>, <code>linkat</code>, <code>symlinkat</code>,
        and <code>unlinkat</code> functions</td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td>Wraps <code>sysconf</code> to support <code>_SC_NPROCESSORS_CONF</code> and
        <code>_SC_NPROCESSORS_ONLN</code></td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td>Wraps <code>sysconf</code> to support <code>_SC_PHYS_PAGES</code></td>
    <td>OSX10.10</td>
  </tr>
</table>

## Building

This project does currently *not* use a configuration phase.

Instead, configuration is supposed to take place by overriding the main
`Makefile`'s variables, either via environment variables, command line
parameters to the `make` call itself or modification of the main `Makefile`.

`GNU make` is a hard build dependency.

Most variables contain paths to various tools. Unless explicitly stated
otherwise, both system (BSD-derived) and `GNU coreutils` variants should work,
with a preference for the native system tools.

### Special variables

#### `PLATFORM`

Major Darwin (not [Mac] OS X/macOS!) version to target against.

This is typically detected automatically, but can also be overridden manually
to test builds for other OS versions.

Some symbols must be built multiple times. Each variant will use a different
data layout and have a special postfix appended to it. The data layouts
supported and needed depend upon the architecture and (target) OS version.

#### `FORCE_ARCH`

Exactly one single architecture to build for.

Older versions of `lipo` do not support the `-archs` flag, so automatic
architecture detection via binary/object file inspection will *not* be possible
on older platforms.

In order to avoid an additional dependency, and, arguably more importantly, a
circular dependency with the `cctools` port providing newer `lipo` versions
within `MacPorts`, this variable was introduced. It disables the automatic
architecture detection feature and instead hardcodes the contained value as the
target architecture.

Within `MacPorts`, we use it in universal (multi-architecture) builds with one
pass per architecture. Eventually, the `MacPorts` system/`muniversal` PortGroup
will merge the resulting binaries into one fat/universal binary automatically.

**This variable takes one single value only. It is not a list.**

If your `lipo` binary is new enough and supports the `-archs` flag, you will
*not* need to use this variable. Instead, directly build the software
universally in one pass via the usual `-arch` compiler flags. The
autodetection, split and merge features will then handle the different
architectures automatically.
