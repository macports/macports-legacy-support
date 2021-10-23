
#include <stdio.h>

int main() {
    dprintf(fileno(stdout), "Hello, %s! My favorite number is %d\n", "Joe", 42);
    return 0;
}
