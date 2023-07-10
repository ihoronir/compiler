int main() {
    int a;
    a = 1;

    int b;
    b = 3;

    if (a) {
        int a;
        a = 4;
        b = b + a;
    }

    return b + a;
}
