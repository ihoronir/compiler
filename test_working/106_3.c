int f(int x) {
    if (x) {
        return 1;
    } else {
        return 0;
    }
}

int main() {
    int a;
    a = 0;
    if (f(1)) a = a + 3;
    if (f(0)) a = a + 4;
    return a;
}
