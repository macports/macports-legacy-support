
#include <stdio.h>
#include <stdarg.h>

int test_vdprintf(int fd, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = vdprintf(fd, format, args);
	va_end(args);
	return ret;
}

int main() {
    dprintf(fileno(stdout), "Hello, %s! My favorite number is %d\n", "Joe", 42);
    test_vdprintf(fileno(stdout), "Hello, %s! My favorite number is %d\n", "Joe", 42);
    return 0;
}
