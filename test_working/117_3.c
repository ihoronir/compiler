int main() {
    int a[2];
    *(a + 1) = 2;
    *a = 1;
    int *p;
    p = a;

    return *p + *(p + 1);
}
