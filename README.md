# MacPorts Support for Legacy OSX Versions

Installs wrapper headers and library functions that add common
functions missing in various older OSX releases to bring them
approximately up to current expected standards.

Three different libraries are provided

 - libMacportsLegacySupport.a      - A static library with the missing or enhanced functions for the given OS.
 - libMacportsLegacySupport.dylib  - A dynamic library with the missing or enhanced functions for the given OS.
 - libMacportsLegacySystem.B.dylib - Similar to libMacportsLegacySupport.dylib but in addition re-exports the symbols from libSystem.B.dylib.

To use this library within [MacPorts](https://github.com/macports)
add the `legacysupport` PortGroup to the Portfile. This will add the
required include paths and libraries to allow the library to do its
magic with most build systems.

Wrapped headers and replaced functions are:

<table>
  <tr>
    <th>Header File</th>
    <th>Feature</th>
    <th>Max Version Needing Feature</th>
  </tr>
  <tr>
    <td><code>assert.h</code></td>
    <td>Adds C11 <code>static_assert</code> definition</td>
    <td>OSX10.10</td>
  </tr>
  <tr>
    <td><code>cmath</code></td>
    <td>Adds the same functions as those provided by the herein <code>math.h</code>,
        in namespace <code>std::</code>.</td>
    <td>see <code>math.h</code></td>
  </tr>
  <tr>
    <td><code>copyfile.h</code></td>
    <td>Completely redone to provide 10.6 version</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td><code>dirent.h</code></td>
    <td>Adds <code>fdopendir</code> function.
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td><code>os/lock.h</code></td>
    <td>Adds <code>os_unfair_lock_lock</code>, <code>os_unfair_lock_trylock</code>, and <code>os_unfair_lock_unlock</code> functions</td>
    <td>OSX10.11</td>
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
    <td rowspan="3"><code>pthread.h</code></td>
    <td>Adds <code>PTHREAD_RWLOCK_INITIALIZER</code></td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td>Adds <code>pthread_setname_np</code> function</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td>Adds <code>pthread_chdir_np</code> and <code>pthread_fchdir_np</code> functions</td>
    <td>OSX10.11</td>
  </tr>
  <tr>
    <td rowspan="3"><code>stdio.h</code></td>
    <td>Adds <code>dprintf</code>, <code>vdprintf</code>, <code>getline</code>, and <code>getdelim</code> functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td>Adds <code>open_memstream</code> and <code>fmemopen</code> functions</td>
    <td>OSX10.12</td>
  </tr>
  <tr>
    <td>Adds include of <code>sys/stdio.h</code></td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td rowspan="4"><code>stdlib.h</code></td>
    <td>Adds <code>posix_memalign</code> functional replacement</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td>Wraps <code>realpath</code> to accept a <code>NULL</code> buffer argument</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td>Fixes non-POSIX <code>realpath</code> in 10.6</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td>Adds <code>arc4random_uniform</code> and <code>arc4random_buf</code> functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td><code>string.h</code></td>
    <td>Adds <code>stpncpy</code>, <code>strnlen</code>, <code>strndup</code> and <code>memmem</code> functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td rowspan="2"><code>strings.h</code></td>
    <td>Adds <code>fls</code>, <code>flsl</code>, and <code>ffsl</code> functions</td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td>Adds <code>flsll</code> and <code>ffsll</code> functions</td>
    <td>OSX10.8</td>
  </tr>
  <tr>
    <td rowspan="3"><code>time.h</code></td>
    <td>Declares <code>asctime_r</code>, <code>ctime_r</code>, <code>gmtime_r</code>, and <code>localtime_r</code> functions that are otherwise hidden in the presence of <code>_ANSI_SOURCE</code>, <code>_POSIX_C_SOURCE</code>, or <code>_XOPEN_SOURCE</code></td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td>Adds functions <code>clock_gettime</code>, <code>clock_gettime_nsec_np</code> and <code>clock_settime</code></td>
    <td>OSX10.11</td>
  </tr>
  <tr>
    <td>Adds function <code>timespec_get</code></td>
    <td>OSX10.14</td>
  </tr>
  <tr>
    <td><code>wchar.h</code></td>
    <td>Adds <code>wcsdup</code>, <code>wcsnlen</code>, <code>wcpcpy</code>,
        <code>wcpncpy</code>, <code>wcscasecmp</code>, and <code>wcsncasecmp</code>
        functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td rowspan="2"><code>mach/mach_time.h</code></td>
    <td>Adds function <code>mach_approximate_time</code></td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td>Adds functions <code>mach_continuous_time</code> and <code>mach_continuous_approximate_time</code></td>
    <td>OSX10.11</td>
  </tr>
  <tr>
    <td><code>mach/machine.h</code></td>
    <td>Adds missing machine definitions</td>
    <td>OSX10.13</td>
  </tr>
  <tr>
    <td><code>net/if.h</code></td>
    <td>Adds include <code>sys/socket.h</code>, expected on current macOS systems</td>
    <td>OSX10.8</td>
  </tr>
  <tr>
    <td><code>net/if_utun.h</code></td>
    <td>Added when missing</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td><code>xlocale/_wchar.h</code></td>
    <td>Adds <code>wcscasecmp_l</code>, <code>wcsncasecmp_l</code> functions</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td><code>sys/aio.h</code></td>
    <td>Adjusts includes and defines to match SDK 10.5+</td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td><code>sys/attr.h</code></td>
    <td>Adds missing <code>VOL_CAP_INT_CLONE</code> definition</td>
    <td>OSX10.11</td>
  </tr>
  <tr>
    <td><code>sys/clonefile.h</code></td>
    <td>Adds <code>clonefile</code>, <code>clonefileat</code>, and <code>fclonefileat</code> functions</td>
    <td>OSX10.11</td>
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
    <td>Adds <code>fsgetpath</code> function</td>
    <td>OSX10.12</td>
  </tr>
  <tr>
    <td><code>sys/mman.h</code></td>
    <td>Adds missing <code>MAP_ANONYMOUS</code> definition</td>
    <td>OSX10.10</td>
  </tr>
  <tr>
    <td rowspan="3"><code>sys/queue.h</code></td>
    <td>Adds <code>SLIST_HEAD_INITIALIZER</code> macro</td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td>Adds <code>SLIST_REMOVE_AFTER</code> macro</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td>Adds <code>STAILQ_FOREACH</code> macro</td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td><code>sys/random.h</code></td>
    <td>Adds <code>getentropy</code> function</td>
    <td>OSX10.11</td>
  </tr>
  <tr>
    <td><code>sys/socket.h</code></td>
    <td>Corrects <code>CMSG_DATA</code> definition</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td rowspan="4"><code>sys/stat.h</code></td>
    <td>Adds <code>fchmodat</code>, <code>fstatat</code>,
        and <code>mkdirat</code> functions</td>
    <td>OSX10.9</td>
  <tr>
    <td>Adds <code>setattrlistat</code> and <code>utimensat</code> functions</td>
    <td>OSX10.12</td>
  </tr>
  <tr>
    <td>Adds <code>lchmod</code> function</td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td>Adds limited <code>stat64</code> support</td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td><code>sys/stdio.h</code></td>
    <td>Adds <code>renameat</code> function</td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td><code>sys/time.h</code></td>
    <td>Adds <code>lutimes</code> function</td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td><code>sys/types.h</code></td>
    <td>Adds definitions for <code>u_char</code>, <code>u_short</code>, <code>u_int</code>, <code>u_long</code>, <code>ushort</code>, and <code>uint</code> types that can be exposed via <code>_DARWIN_C_SOURCE</code></td>
    <td>OSX10.4</td>
  </tr>
  <tr>
    <td rowspan="3"><code>sys/unistd.h</code></td>
    <td>Adds <code>getattrlistat</code>, <code>readlinkat</code>, <code>faccessat</code>,
        <code>fchownat</code>, <code>linkat</code>, <code>symlinkat</code>,
        <code>unlinkat</code>,
        <code>fsetattrlist</code>, and <code>fgetattrlist</code> functions.</td>
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
  <tr>
    <td><code>uuid/uuid.h</code></td>
    <td>Adds typedef of <code>uuid_string_t</code></td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td><code>CoreFoundation/CoreFoundation.h</code></td>
    <td>Adds <code>CFPropertyListCreateWithStream</code> function</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td><code>OpenGL/gliDispatch.h</code></td>
    <td>Wraps <code>gliDispatch.h</code> to prevent including
        <code>glext.h</code> and thereby match behaviour of newer systems.</td>
    <td>OSX10.6</td>
  </tr>
  <tr>
    <td><code>TargetConditionals.h</code></td>
    <td>Adds definitions for all TARGET_* definitions as listed in 15.x SDK, if needed.</td>
    <td>???</td>
  </tr>
  <tr>
    <td><code>-</code></td>
    <td>Adds <code>__bzero</code> library symbol</td>
    <td>OSX10.5</td>
  </tr>
  <tr>
    <td><code>-</code></td>
    <td>Adds <code>_dirfd</code> library symbol</td>
    <td>OSX10.7</td>
  </tr>
  <tr>
    <td><code>-</code></td>
    <td>Adds <code>_fstatat$INODE64</code> library symbol</td>
    <td>OSX10.9</td>
  </tr>
  <tr>
    <td><code>-</code></td>
    <td>Provides a workaround for bug in <code>pthread_get_stacksize_np</code></td>
    <td>OSX10.4, OSX10.5, OSX10.9, OSX10.10</td>
  </tr>
  <tr>
    <td><code>-</code></td>
    <td>Fixes boottime bug in 64-bit <code>sysctl()</code> and <code>sysctlbyname()</code></td>
    <td>OSX10.5</td>
  </tr>
</table>

For information on building this library outside MacPorts, see BUILDING.txt.
