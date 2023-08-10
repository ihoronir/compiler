int ptrdiff();
struct A {
    int a;
    char c;
    char d;
    int b;
};
int main() {
    struct A *p;
    return ptrdiff(p + 1, p);
}
