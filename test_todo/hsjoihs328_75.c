struct A {
    int a;
    int b;
    int *q;
    int *t;
    int *p;
};
struct A f(void) {
    struct A u;
    u.a = 100;
    return u;
}
int main(void) {
    struct A u = f();
    struct A v;
    v.a = 1;
    return (0 ? u : v).a + 74;
}
