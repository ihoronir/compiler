int *alloc4();
int main() {
    int *p;
    int c;
    c = -2;
    int d;
    d = 1;
    p = alloc4(62, 8, 31, 85);
    int *q;
    q = p - c;
    return *(d + q) - *q + (*p - 2) * 2;
}
