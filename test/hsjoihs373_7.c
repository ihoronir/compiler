void foo(int*p) {*p=7;} int main() { int a; if (0) { a = 3; } else { foo(&a); } return a; }
