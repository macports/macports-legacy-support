
# Support headers for legacy OSX versions

Installs a number of wrapper headers around system headers that add
functionality missing in various older OSX releases.

Installed headers use the `include_next` pre-processor feature to add
the missing features and then forward include the original header.
So using these headers instead of the originals should be transparent.

 - time.h - Adds an implementation of clock_gettime for pre-OSX10.12
 