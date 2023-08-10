struct A {
    int k[15];
};
int main() {
    struct A s;
    void *p = s.k;
    return 35;
}
