int *alloc4();
int main() {
    int *p;
    int *r;
    p = alloc4(62, 8, 31, 85);
    int *q;
    q = p;
    return *q;
}
