int foo() { return 2; }
int bar() { return 7; }
int main() {
    int a;
    int b;
    int c;
    a = 3;
    b = 5;
    c = 2;
    if (a)
        if (0)
            b = foo();
        else
            c = bar();
    return 162 + b + c;
}
