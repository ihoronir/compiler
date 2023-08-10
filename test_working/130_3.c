int a;
int b;

int *f() {
    b += 2;
    return &a;
}

int main() {
    *f() += 1;
    return a + b;
}
