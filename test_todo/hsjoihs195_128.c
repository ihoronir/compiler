struct A {
    int a;
    int *b;
    int c;
};
struct B {
    char d;
    struct A e;
};
int main() { return sizeof(struct B[4]); }
