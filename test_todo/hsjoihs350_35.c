struct A {
    int k[15];
};
int main() {
    struct A s;
    int(*p)[15] = &s.k;
    return 35;
}
