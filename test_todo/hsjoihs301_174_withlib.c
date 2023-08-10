struct A {
    int a;
    char c;
    char d;
    int b;
};
struct A *get_struct_pointer();
int main() {
    struct A a = *get_struct_pointer(100, 74);
    return a.a + a.b;
}
