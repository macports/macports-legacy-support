
/* Not very exciting but normally this inclusion will fail with GCC 4.5+ pre-10.15 */
#include <IOKit/usb/USB.h>
#include <stdio.h>

int main() {
    printf("Including <IOKit/usb/USB.h> succeeded\n");
    return 0;
}
