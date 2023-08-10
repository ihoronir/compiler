int foo(char a, char b) {
    int d;
    d = 3;
    char c;
    c = a + d;
    return c * b;
}
int main() {
    char f;
    f = 3;
    return foo(f, 4) + 150;
}
