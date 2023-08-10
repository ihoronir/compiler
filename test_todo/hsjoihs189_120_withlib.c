int ptrdiff();
struct A {
    int a;
    int *b;
    int c;
};
struct A a[1][5];
int main() { return ptrdiff(a + 1, a); }
