int main() {
    int a = 0;
    int *b = a ? 0 : &a;
    return 123;
}
