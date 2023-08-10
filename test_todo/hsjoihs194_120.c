struct A {
    int a;
    int *b;
    int c;
};
int main() { return sizeof(struct A[5]); }
