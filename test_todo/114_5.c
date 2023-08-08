int **f(int **x) { return x; }

int main() {
    int a;
    a = 5;
    int *b;
    b = &a;
    int **c;
    c = &b;

    int **d;
    d = f(c);

    return **d;
}
