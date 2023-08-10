int foobar();
int a(int N) { return 3; }
int main() {
    int i;
    foobar("%d %d", i, a(i));
    return 0;
}
