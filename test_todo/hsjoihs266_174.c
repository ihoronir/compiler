int main(void) {
    int a[5];
    a[1] = 174;
    int *p = a + 3;
    p -= 2;
    return *p;
}
