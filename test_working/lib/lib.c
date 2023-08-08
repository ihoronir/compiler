#include <stdlib.h>

int *alloc4(int a, int b, int c, int d) {
    int *p = calloc(4, sizeof(int));
    p[0] = a;
    p[1] = b;
    p[2] = c;
    p[3] = d;
    return p;
}
