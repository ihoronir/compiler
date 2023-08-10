struct A {
    int a;
    int *b;
    int c;
};
int main() { return _Alignof(struct A[5]); }
