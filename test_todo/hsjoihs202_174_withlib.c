struct A {
    int a;
    char c;
    char d;
    int b;
};
struct A *get_struct_pointer();
int main() {
    struct A *p;
    p = get_struct_pointer(100, 74);
    return p->a + p->b;
}
