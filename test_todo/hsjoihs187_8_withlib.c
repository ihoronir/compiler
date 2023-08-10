int ptrdiff();
struct A {
    int a;
    int b;
};
int main() {
    struct A *p;
    return ptrdiff(p + 1, p);
}
